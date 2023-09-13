/*
  This file is part of the continued NoLifeStory project
  Copyright (C) 2014-2023  Peter Atashian, Ryan Payton, Patrik Rogalski


  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.


  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.


  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <lz4.h>
#include <lz4hc.h>
#include <zlib.h>

#include <squish.h>

#include <algorithm>
#include <array>
#include <codecvt>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace nl {

// Some typedefs
typedef char char8_t;
typedef uint32_t id_t;
typedef uint8_t key_t;
typedef int32_t int_t;
// The keys
// TODO - Use AES to generate these keys at runtime
extern key_t key_gms[65536];
extern key_t key_kms[65536];
key_t const* keys[2] = { key_gms, key_kms };

// Tables for color lookups
uint8_t const color_lookup_tables[3][72] = {
    // table4
    { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF },
    // table5
    { 0x00, 0x08, 0x10, 0x19, 0x21, 0x29, 0x31, 0x3A, 0x42, 0x4A, 0x52,
        0x5A, 0x63, 0x6B, 0x73, 0x7B, 0x84, 0x8C, 0x94, 0x9C, 0xA5, 0xAD,
        0xB5, 0xBD, 0xC5, 0xCE, 0xD6, 0xDE, 0xE6, 0xEF, 0xF7, 0xFF },
    // table6
    { 0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28,
        0x2D, 0x31, 0x35, 0x39, 0x3D, 0x41, 0x45, 0x49, 0x4D, 0x51, 0x55,
        0x59, 0x5D, 0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D, 0x82,
        0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A, 0x9E, 0xA2, 0xA6, 0xAA, 0xAE,
        0xB2, 0xB6, 0xBA, 0xBE, 0xC2, 0xC6, 0xCA, 0xCE, 0xD2, 0xD7, 0xDB,
        0xDF, 0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB, 0xFF }
};

// cp1252 table
char16_t const cp1252[0x100] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,
    0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010, 0x0011,
    0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A,
    0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C,
    0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
    0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E,
    0x003F, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050,
    0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0060, 0x0061, 0x0062,
    0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B,
    0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D,
    0x007E, 0x007F, 0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020,
    0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
    0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC,
    0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178, 0x00A0, 0x00A1,
    0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA,
    0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1, 0x00B2, 0x00B3,
    0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC,
    0x00BD, 0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5,
    0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE,
    0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0,
    0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9,
    0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2,
    0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB,
    0x00FC, 0x00FD, 0x00FE, 0x00FF
};
// Identity operation because C++ doesn't have such a template. Surprising, I
// know.
template <typename T>
struct identity {
    T const& operator()(T const& v) const { return v; }
};

template <int N>
void scale(std::vector<uint8_t> const& input, std::vector<uint8_t>& output,
    int width, int height)
{
    auto in = reinterpret_cast<uint32_t const*>(input.data());
    auto out = reinterpret_cast<uint32_t*>(output.data());
    auto w = width / N;
    auto h = height / N;
    for (auto y = 0; y < h; ++y) {
        for (auto x = 0; x < w; ++x) {
            auto p = in[y * w + x];
            for (auto yy = y * N; yy < (y + 1) * N; ++yy) {
                for (auto xx = x * N; xx < (x + 1) * N; ++xx) {
                    out[yy * width + xx] = p;
                }
            }
        }
    }
}

// Input memory mapped file
struct imapfile {
    char const* base = nullptr;
    char const* offset = nullptr;
    int file_handle = 0;
    size_t file_size = 0;
    void open(std::string p)
    {
        file_handle = ::open(p.c_str(), O_RDONLY);
        if (file_handle == -1)
            throw std::runtime_error("Failed to open file " + p);
        struct stat finfo;
        if (fstat(file_handle, &finfo) == -1)
            throw std::runtime_error("Failed to obtain file information of file " + p);
        file_size = finfo.st_size;
        base = reinterpret_cast<char const*>(
            mmap(nullptr, file_size, PROT_READ, MAP_SHARED, file_handle, 0));
        if (reinterpret_cast<intptr_t>(base) == -1)
            throw std::runtime_error("Failed to create memory mapping of file " + p);
        offset = base;
    }
    ~imapfile()
    {
        munmap(const_cast<char*>(base), file_size);
        close(file_handle);
    }
    size_t tell() { return static_cast<size_t>(offset - base); }
    void seek(size_t n) { offset = base + n; }
    void skip(size_t n) { offset += n; }
    template <typename T>
    T read()
    {
        auto& v = *reinterpret_cast<T const*>(offset);
        offset += sizeof(T);
        return v;
    }
    int32_t read_cint()
    {
        int8_t a = read<int8_t>();
        return a != -128 ? a : read<int32_t>();
    }
};
// Output memory mapped file
struct omapfile {
    char* base = nullptr;
    char* offset = nullptr;
    int file_handle = 0;
    size_t file_size = 0;
    void open(std::string p, uint64_t size)
    {
        file_handle = ::open(p.c_str(), O_RDWR | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (file_handle == -1)
            throw std::runtime_error("Failed to open file " + p);
        file_size = size;
        if (::lseek(file_handle, file_size - 1, SEEK_SET) == -1)
            throw std::runtime_error("Error calling lseek() to 'stretch' file " + p);
        if (::write(file_handle, "", 1) != 1)
            throw std::runtime_error("Error writing last byte of file " + p);
        base = reinterpret_cast<char*>(::mmap(nullptr, file_size,
            PROT_READ | PROT_WRITE, MAP_SHARED,
            file_handle, 0));
        if (reinterpret_cast<intptr_t>(base) == -1)
            throw std::runtime_error("Failed to create memory mapping of file " + p);
        offset = base;
    }
    void close()
    {
        ::munmap(const_cast<char*>(base), file_size);
        ::close(file_handle);
    }
    size_t tell() { return static_cast<size_t>(offset - base); }
    void seek(size_t n) { offset = base + n; }
    void skip(size_t n) { offset += n; }
    template <typename T>
    void write(T const& v)
    {
        *reinterpret_cast<T*>(offset) = v;
        offset += sizeof(T);
    }
    void write(void const* buf, size_t size)
    {
        std::memcpy(offset, buf, size);
        offset += size;
    }
};
// Node stuff
#pragma pack(push, 1)
struct node {
    enum class type : uint16_t {
        none = 0,
        integer = 1,
        real = 2,
        string = 3,
        vector = 4,
        bitmap = 5,
        audio = 6,
        uol = 7
    };
    uint32_t name = 0;
    uint32_t children = 0;
    uint16_t num = 0;
    type data_type = type::none;
    union _data {
        int64_t integer = 0;
        double real;
        uint32_t string;
        int32_t vector[2];
        struct _bitmap {
            uint32_t id;
            uint16_t width;
            uint16_t height;
        } bitmap;
        struct _audio {
            uint32_t id;
            uint32_t length;
        } audio;
    } data;
};
#pragma pack(pop)
struct audio {
    uint32_t length;
    uint64_t data;
};
struct bitmap {
    uint64_t data;
    uint8_t const* key;
};
// The main class itself
struct wztonx {
    // Variables
    File file;
    imapfile in;
    omapfile out;
    std::vector<node> nodes = std::vector<node> { { node {} } };
    std::vector<std::pair<id_t, id_t>> nodes_to_sort;
    std::unordered_map<uint32_t, id_t, identity<uint32_t>> string_map;
    std::vector<std::string> strings;
    std::string str_buf;
    std::u16string wstr_buf;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    char8_t const* u8key = nullptr;
    char16_t const* u16key = nullptr;
    std::vector<std::pair<id_t, int32_t>> imgs;
    size_t file_start = 0;
    std::vector<id_t> uol_path;
    std::vector<std::vector<id_t>> uols;
    std::vector<id_t> link_path;
    std::vector<std::vector<id_t>> links;
    std::vector<bitmap> bitmaps;
    std::vector<audio> audios;
    size_t offset, node_offset, string_offset, string_table_offset, bitmap_offset,
        bitmap_table_offset, audio_offset, audio_table_offset;
    bool client, hc;
    std::string wzfilename, nxfilename;
    // Methods
    std::string convert_str(std::u16string const& p_str)
    {

        auto ptr = reinterpret_cast<wchar_t const*>(p_str.c_str());
        return convert.to_bytes(ptr, ptr + p_str.size());
    }

    id_t add_string(std::string str)
    {
        uint32_t hash = 2166136261u;
        for (auto c : str) {
            hash ^= c;
            hash *= 16777619u;
        }
        auto& id = string_map[hash];
        if (id != 0)
            return id;
        id = static_cast<id_t>(strings.size());
        strings.push_back(std::move(str));
        return id;
    }
    id_t read_enc_string()
    {
        auto len = in.read<int8_t>();
        if (len > 0) {
            auto slen = len == 127 ? in.read<uint32_t>() : len;
            auto ows = reinterpret_cast<char16_t const*>(in.offset);
            in.skip(slen * 2u);
            auto mask = 0xAAAAu;
            wstr_buf.resize(slen);
            for (auto i = 0u; i < std::min(slen, 0x10000u); ++i) {
                wstr_buf[i] = static_cast<char16_t>(ows[i] ^ u16key[i] ^ mask);
                ++mask;
            }
            for (auto i = 0x10000u; i < slen; ++i) {
                wstr_buf[i] = static_cast<char16_t>(ows[i] ^ mask);
                ++mask;
            }
            return add_string(convert_str(wstr_buf));
        }
        if (len < 0) {
            auto slen = len == -128 ? in.read<uint32_t>() : -len;
            auto os = reinterpret_cast<char8_t const*>(in.offset);
            in.skip(slen);
            auto mask = 0xAAu;
            str_buf.resize(slen);
            for (auto i = 0u; i < std::min(slen, 0x10000u); ++i) {
                str_buf[i] = static_cast<char8_t>(os[i] ^ u8key[i] ^ mask);
                ++mask;
            }
            for (auto i = 0x10000u; i < slen; ++i) {
                str_buf[i] = static_cast<char8_t>(os[i] ^ mask);
                ++mask;
            }
            if (std::any_of(str_buf.begin(), str_buf.end(), [](char const& c) {
                    return static_cast<uint8_t>(c) >= 0x80;
                })) {
                std::transform(
                    str_buf.cbegin(), str_buf.cend(), std::back_inserter(wstr_buf),
                    [](char c) { return cp1252[static_cast<unsigned char>(c)]; });
                return add_string(convert_str(wstr_buf));
            }
            return add_string(str_buf);
        }
        return 0;
    }
    id_t read_prop_string(size_t p_offset)
    {
        auto a = in.read<uint8_t>();
        switch (a) {
        case 0x00:
        case 0x73:
            return read_enc_string();
        case 0x01:
        case 0x1B: {
            auto o = in.read<int32_t>() + p_offset;
            auto p = in.tell();
            in.seek(o);
            auto s = read_enc_string();
            in.seek(p);
            return s;
        }
        default:
            throw std::runtime_error("Unknown property string type: " + std::to_string(a));
        }
    }
    void deduce_key()
    {
        auto len = in.read<int8_t>();
        if (len >= 0)
            throw std::runtime_error("I give up");
        auto slen = len == -128 ? in.read<uint32_t>() : -len;
        u8key = nullptr;
        for (auto key : keys) {
            auto os = reinterpret_cast<char8_t const*>(in.offset);
            uint8_t mask = 0xAA;
            auto k = reinterpret_cast<char8_t const*>(key);
            bool valid = true;
            for (auto i = 0u; i < slen; ++i, ++mask) {
                auto c = static_cast<uint8_t>(os[i] ^ k[i] ^ mask);
                if (c < 0x20 || c >= 0x80) {
                    valid = false;
                }
            }
            if (valid) {
                u8key = reinterpret_cast<char8_t const*>(key);
                u16key = reinterpret_cast<char16_t const*>(key);
            }
        }
        if (!u8key)
            throw std::runtime_error("Failed to identify the locale");
        in.skip(slen);
    }
    void sort_nodes(id_t first, id_t count)
    {
        std::sort(nodes.begin() + first, nodes.begin() + first + count,
            [this](node const& n1, node const& n2) {
                return strings[n1.name] < strings[n2.name];
            });
    }
    void find_uols(id_t uol_node)
    {
        auto& n = nodes[uol_node];
        if (n.data_type == node::type::uol) {
            uol_path.push_back(uol_node);
            uols.push_back(uol_path);
            uol_path.pop_back();
        } else if (n.num != 0) {
            uol_path.push_back(uol_node);
            for (auto i = 0u; i < n.num; ++i)
                find_uols(n.children + i);
            uol_path.pop_back();
        }
    }
    // std::vector<std::string> name_path;
    void find_links(id_t link_node, std::string const& str)
    {
        auto& n = nodes[link_node];
        auto& s = strings[n.name];
        if (s == str) {
            link_path.push_back(link_node);
            // name_path.push_back(s);
            links.push_back(link_path);
            link_path.pop_back();
            // name_path.pop_back();
        } else if (n.num != 0) {
            link_path.push_back(link_node);
            // name_path.push_back(s);
            for (auto i = 0u; i < n.num; ++i)
                find_links(n.children + i, str);
            link_path.pop_back();
            // name_path.pop_back();
        }
    }
    id_t get_child(id_t parent_node, std::string const& str)
    {
        if (parent_node == 0)
            return 0;
        auto& n = nodes[parent_node];
        auto first = nodes.begin() + n.children;
        auto last = first + n.num;
        auto it = std::lower_bound(first, last, str,
            [this](node const& n, std::string const& s) {
                return strings[n.name] < s;
            });
        if (it == last)
            return 0;
        if (strings[it->name] != str)
            return 0;
        return static_cast<id_t>(it - nodes.begin());
    }
    id_t get_child_full(id_t parent_node, std::string const& str)
    {
        auto& n = nodes[parent_node];
        auto first = nodes.begin() + n.children;
        auto last = first + n.num;
        auto it = std::lower_bound(first, last, str,
            [this](node const& n, std::string const& s) {
                return strings[n.name] < s;
            });
        if (it == last)
            return 0;
        if (strings[it->name] != str)
            return 0;
        return static_cast<id_t>(it - nodes.begin());
    }
    bool resolve_uol(std::vector<id_t> uol)
    {
        auto& n = nodes[uol.back()];
        uol.pop_back();
        if (n.data_type != node::type::uol)
            throw std::runtime_error("Welp. I failed.");
        auto& s = strings[n.data.string];
        auto b = 0u;
        for (auto i = 0u; i < s.size(); ++i)
            if (s[i] == '/') {
                if (i - b == 2 && s[b] == '.' && s[b + 1] == '.')
                    uol.pop_back();
                else
                    uol.push_back(get_child(uol.back(), s.substr(b, i - b)));
                b = ++i;
            }
        uol.push_back(get_child(uol.back(), s.substr(b)));
        if (uol.back() == 0)
            return false;
        auto& nr = nodes[uol.back()];
        if (nr.data_type == node::type::uol)
            return false;
        n.data_type = nr.data_type;
        n.children = nr.children;
        n.num = nr.num;
        n.data.integer = nr.data.integer;
        return true;
    }
    bool resolve_source(std::vector<id_t> link)
    {
        auto& n = nodes[link.back()];
        link.pop_back();
        auto& s = strings[n.data.string];
        std::istringstream stream(s);
        std::vector<std::string> parts;
        std::string segment;
        while (std::getline(stream, segment, '/'))
            parts.push_back(segment);
        id_t r = 0;
        for (auto const& part : parts)
            r = get_child_full(r, part);
        if (r == 0)
            return false;
        auto& nr = nodes[r];
        auto& pn = nodes[link.back()];
        pn.data = nr.data;
        return true;
    }
    bool resolve_outlink(std::vector<id_t> link)
    {
        auto& n = nodes[link.back()];
        link.pop_back();
        auto& s = strings[n.data.string];
        std::istringstream stream(s);
        std::vector<std::string> parts;
        std::string segment;
        while (std::getline(stream, segment, '/'))
            parts.push_back(segment);
        if (parts[0] == "Map")
            return true;
        id_t r = 0;
        for (auto const& part : parts)
            r = get_child_full(r, part);
        if (r == 0)
            return false;
        auto& nr = nodes[r];
        auto& pn = nodes[link.back()];
        pn.data = nr.data;
        return true;
    }
    bool resolve_inlink(std::vector<id_t> link)
    {
        auto& n = nodes[link.back()];
        link.pop_back();
        auto& s = strings[n.data.string];
        std::istringstream stream(s);
        std::vector<std::string> parts;
        std::string segment;
        while (std::getline(stream, segment, '/'))
            parts.push_back(segment);
        auto r = link.back();
        auto& pn = nodes[r];
        for (;;) {
            // auto & p = nodes[r];
            // auto & pn = strings[p.name];
            for (auto const& part : parts) {
                r = get_child_full(r, part);
                if (r == 0)
                    break;
            }
            if (r != 0)
                break;
            link.pop_back();
            if (link.size() == 0)
                break;
            r = link.back();
        }
        if (r == 0)
            return false;
        auto& nr = nodes[r];
        pn.data = nr.data;
        return true;
    }
    void uol_fail(std::vector<id_t>& uol)
    {
        // std::cerr << "Invalid UOL: ";
        // for (auto id : uol) {
        // auto & n = nodes[id];
        // std::cerr << '/' << strings[n.name];
        //}
        auto& n = nodes[uol.back()];
        if (n.data_type == node::type::uol) {
            // std::cerr << " = \"" << strings[n.data.string] << "\"" << std::endl;
            //  If we failed to resolve any uols, just turn them into useless empty
            //  nodes
            n.data_type = node::type::none;
        } else {
            std::cerr << " claims to be an invalid UOL but isn't a UOL???"
                      << std::endl;
        }
    }
    void source_fail(std::vector<id_t>& link, std::string const& str)
    {
        auto& n = nodes[link.back()];
        std::cerr << "Failed to find " << str << " for [" << strings[n.data.string]
                  << "]." << std::endl;
    }
    void directory(id_t dir_node)
    {
        std::vector<id_t> directories;
        auto& n = nodes[dir_node];
        auto count = static_cast<id_t>(in.read_cint());
        auto ni = static_cast<id_t>(nodes.size());
        n.num = static_cast<uint16_t>(count);
        n.children = ni;
        nodes.resize(ni + count);
        for (auto i = 0u; i < count; ++i) {
            auto& nn = nodes[ni + i];
            auto type = in.read<uint8_t>();
            switch (type) {
            case 1:
                throw std::runtime_error("Found the elusive type 1 directory");
            case 2: {
                auto s = in.read<int32_t>();
                auto p = in.tell();
                in.seek(file_start + s);
                type = in.read<uint8_t>();
                nn.name = read_enc_string();
                in.seek(p);
                break;
            }
            case 3:
            case 4:
                nn.name = read_enc_string();
                break;
            default:
                throw std::runtime_error("Unknown directory type");
            }
            auto size = in.read_cint();
            if (size < 0)
                throw std::runtime_error("Directory/img has invalid size!");
            in.read_cint(); // Offset that nobody cares about
            in.skip(4); // Checksum that nobody cares about
            if (type == 3)
                directories.push_back(ni + i);
            else if (type == 4)
                imgs.emplace_back(ni + i, size);
            else
                throw std::runtime_error("Unknown type 2 directory");
        }
        for (auto it : directories)
            directory(it);
        nodes_to_sort.emplace_back(ni, count);
    }
    void extended_property(id_t prop_node, size_t p_offset)
    {
        auto& n = nodes[prop_node];
        auto& st = strings[read_prop_string(p_offset)];
        if (st == "Property") {
            in.skip(2);
            sub_property(prop_node, p_offset);
        } else if (st == "Canvas") {
            in.skip(1);
            if (in.read<uint8_t>() == 1) {
                in.skip(2);
                sub_property(prop_node, p_offset);
            }
            // Have to recalculate n, because address may have changed
            // because sub_property may increase the size of nodes
            auto& nn = nodes[prop_node];
            nn.data_type = node::type::bitmap;
            nn.data.bitmap.id = static_cast<uint32_t>(bitmaps.size());
            bitmaps.push_back({ in.tell(), reinterpret_cast<uint8_t const*>(u8key) });
            nn.data.bitmap.width = static_cast<uint16_t>(in.read_cint());
            nn.data.bitmap.height = static_cast<uint16_t>(in.read_cint());
        } else if (st == "Shape2D#Vector2D") {
            n.data_type = node::type::vector;
            n.data.vector[0] = in.read_cint();
            n.data.vector[1] = in.read_cint();
        } else if (st == "Shape2D#Convex2D") {
            auto count = static_cast<id_t>(in.read_cint());
            auto ni = static_cast<id_t>(nodes.size());
            n.num = static_cast<uint16_t>(count);
            n.children = ni;
            nodes.resize(nodes.size() + count);
            for (auto i = 0u; i < count; ++i) {
                auto& nn = nodes[ni + i];
                auto es = std::to_string(i);
                nn.name = add_string(std::move(es));
                extended_property(ni, p_offset);
            }
            nodes_to_sort.emplace_back(ni, count);
        } else if (st == "Sound_DX8") {
            n.data_type = node::type::audio;
            n.data.audio.id = static_cast<uint32_t>(audios.size());
            audio a;
            in.skip(1); // Always 0
            a.length = static_cast<uint32_t>(in.read_cint()) + 82u;
            n.data.audio.length = a.length;
            in.read_cint();
            a.data = in.tell();
            audios.push_back(a);
        } else if (st == "UOL") {
            in.skip(1);
            n.data_type = node::type::uol;
            n.data.string = read_prop_string(p_offset);
        } else {
            throw std::runtime_error("Unknown sub property type: " + st);
        }
    }
    void sub_property(id_t prop_node, size_t p_offset)
    {
        /*
         * TODO: terminate called after throwing an instance of 'std::range_error'
         *         what():  wstring_convert::to_bytes
         *       Aborted
         */
        auto& n = nodes[prop_node];
        auto count = static_cast<id_t>(in.read_cint());
        auto ni = static_cast<id_t>(nodes.size());
        n.num = static_cast<uint16_t>(count);
        n.children = ni;
        nodes.resize(nodes.size() + count);
        for (auto i = 0u; i < count; ++i) {
            auto& nn = nodes[ni + i];
            nn.name = read_prop_string(p_offset);
            auto type = in.read<uint8_t>();
            uint8_t num;
            size_t p;
            switch (type) {
            case 0x00: // Turning null nodes into integers with an id. Useful for
                       // zmap.img
                nn.data_type = node::type::integer;
                nn.data.integer = i;
                break;
            case 0x0B: // TODO: Check if 0x0B really is a 16bit int
            case 0x02:
                nn.data_type = node::type::integer;
                nn.data.integer = in.read<uint16_t>();
                break;
            case 0x03:
                nn.data_type = node::type::integer;
                nn.data.integer = in.read_cint();
                break;
            case 0x04:
                nn.data_type = node::type::real;
                num = in.read<uint8_t>();
                if (num == 0x80) {
                    nn.data.real = in.read<float>();
                } else {
                    nn.data.real = static_cast<int8_t>(num);
                }
                break;
            case 0x05:
                nn.data_type = node::type::real;
                nn.data.real = in.read<double>();
                break;
            case 0x08:
                nn.data_type = node::type::string;
                nn.data.string = read_prop_string(p_offset);
                break;
            case 0x09:
                p = in.read<int32_t>() + in.tell();
                extended_property(ni + i, p_offset);
                in.seek(p);
                break;
            case 0x13:
                nn.data_type = node::type::integer;
                nn.data.integer = in.read_cint();
                break;
            case 0x14:
                nn.data_type = node::type::integer;
                num = in.read<uint8_t>();
                if (num == 0x80) {
                    nn.data.integer = in.read<int64_t>();
                } else {
                    nn.data.integer = static_cast<int8_t>(num);
                }
                break;
            default:
                throw std::runtime_error("Unknown sub property type: " + std::to_string(type));
            }
        }
        nodes_to_sort.emplace_back(ni, count);
    }
    void img(id_t img_node, int32_t size)
    {
        auto p = in.tell();
        auto n1 = in.read<uint8_t>();
        if (n1 == 1) {
            lua_script(img_node);
        } else {
            deduce_key();
            in.seek(p);
            extended_property(img_node, p);
        }
        std::cerr << "in.seek();" << std::endl;
        in.seek(p + size);
    }
    void lua_script(id_t script_node)
    {
        auto slen = static_cast<uint32_t>(in.read_cint());
        if (slen > 0x1ffff)
            throw std::runtime_error("Lua script is too long");
        auto os = reinterpret_cast<char8_t const*>(in.offset);
        str_buf.resize(slen);
        auto key = key_kms;
        u8key = reinterpret_cast<char8_t const*>(key);
        for (auto i = 0u; i < slen; ++i) {
            str_buf[i] = static_cast<char8_t>(os[i] ^ u8key[i]);
        }
        auto string = add_string(str_buf);
        auto& n = nodes[script_node];
        n.data_type = node::type::string;
        n.data.string = string;
    }
    virtual void parse_file()
    {
        std::cerr << "Working on " << wzfilename << std::endl;
        std::cout << "Parsing input......." << std::endl;
        m_file = File::File(m_wzfilename);
        // in.open(wzfilename);

        auto magic = in.read<uint32_t>();
        if (magic != 0x31474B50)
            throw std::runtime_error("Not a valid WZ file");
        in.skip(8);
        file_start = in.read<uint32_t>();
        // Just skip the copyright string
        in.seek(file_start + 2);
        in.read_cint();
        in.skip(1);
        deduce_key();
        in.seek(file_start + 2);
        add_string({});
        directory(0);
        for (auto& it : imgs)
            img(it.first, it.second);
        std::cout << "Done!" << std::endl;
        finish_parse();
    }
    void finish_parse()
    {
        for (auto const& n : nodes_to_sort)
            sort_nodes(n.first, n.second);
        std::cout << "Parsing uol.........";
        // uol
        find_uols(0);
        for (;;) {
            auto it = std::remove_if(
                uols.begin(), uols.end(),
                [this](std::vector<id_t> const& v) { return resolve_uol(v); });
            auto diff = uols.end() - it;
            uols.erase(it, uols.end());
            if (diff == 0)
                break;
        }
        for (auto& it : uols)
            uol_fail(it);
        std::cout << "Done!" << std::endl;
        // source
        std::cout << "Parsing source......";
        find_links(0, "source");
        for (;;) {
            auto it = std::remove_if(
                links.begin(), links.end(),
                [this](std::vector<id_t> const& v) { return resolve_source(v); });
            auto diff = links.end() - it;
            links.erase(it, links.end());
            if (diff == 0)
                break;
        }
        for (auto& it : links)
            source_fail(it, "source");
        links.clear();
        std::cout << "Done!" << std::endl;
        //_outlink
        std::cout << "Parsing _outlink....";
        find_links(0, "_outlink");
        for (;;) {
            auto it = std::remove_if(
                links.begin(), links.end(),
                [this](std::vector<id_t> const& v) { return resolve_outlink(v); });
            auto diff = links.end() - it;
            links.erase(it, links.end());
            if (diff == 0)
                break;
        }
        for (auto& it : links)
            source_fail(it, "_outlink");
        links.clear();
        std::cout << "Done!" << std::endl;
        //_inlink
        std::cout << "Parsing _inlink.....";
        find_links(0, "_inlink");
        for (;;) {
            auto it = std::remove_if(
                links.begin(), links.end(),
                [this](std::vector<id_t> const& v) { return resolve_inlink(v); });
            auto diff = links.end() - it;
            links.erase(it, links.end());
            if (diff == 0)
                break;
        }
        for (auto& it : links)
            source_fail(it, "_inlink");
        links.clear();
        std::cout << "Done!" << std::endl;
    }
    void calculate_offsets()
    {
        offset = 0;
        offset += 52;
        offset += 0x10 - (offset & 0xf);
        node_offset = offset;
        offset += nodes.size() * 20;
        offset += 0x10 - (offset & 0xf);
        string_table_offset = offset;
        offset += strings.size() * 8;
        offset += 0x10 - (offset & 0xf);
        string_offset = offset;
        offset += std::accumulate(strings.begin(), strings.end(), 0ull,
            [](size_t n, std::string const& s) {
                return n + s.size() + 2 + (s.size() & 1 ? 1 : 0);
            });
        offset += 0x10 - (offset & 0xf);
        audio_table_offset = offset;
        if (client) {
            offset += audios.size() * 8;
            offset += 0x10 - (offset & 0xf);
        }
        bitmap_table_offset = offset;
        if (client) {
            offset += bitmaps.size() * 8;
            offset += 0x10 - (offset & 0xf);
        }
        audio_offset = offset;
        if (client) {
            offset += std::accumulate(
                audios.begin(), audios.end(), 0ull,
                [](size_t n, audio const& a) { return n + a.length; });
            offset += 0x10 - (offset & 0xf);
        }
        bitmap_offset = offset;
    }
    void open_output()
    {
        std::cout << "Opening output......";
        calculate_offsets();
        out.open(nxfilename, offset);
        out.seek(0);
        out.write<uint32_t>(0x34474B50);
        out.write<uint32_t>(static_cast<uint32_t>(nodes.size()));
        out.write<uint64_t>(node_offset);
        out.write<uint32_t>(static_cast<uint32_t>(strings.size()));
        out.write<uint64_t>(string_table_offset);
        if (client) {
            out.write<uint32_t>(static_cast<uint32_t>(bitmaps.size()));
            out.write<uint64_t>(bitmap_table_offset);
            out.write<uint32_t>(static_cast<uint32_t>(audios.size()));
            out.write<uint64_t>(audio_table_offset);
        } else {
            out.write<uint32_t>(0);
            out.write<uint64_t>(0);
            out.write<uint32_t>(0);
            out.write<uint64_t>(0);
        }
        std::cout << "Done!" << std::endl;
    }
    void write_nodes()
    {
        std::cout << "Writing nodes.......";
        out.seek(node_offset);
        out.write(nodes.data(), nodes.size() * 20);
        std::cout << "Done!" << std::endl;
    }
    void write_strings()
    {
        std::cout << "Writing strings.....";
        out.seek(string_table_offset);
        auto next_str = string_offset;
        for (auto const& s : strings) {
            out.write<uint64_t>(next_str);
            next_str += s.size() + 2;
            if (s.size() & 1)
                ++next_str;
        }
        out.seek(string_offset);
        for (auto const& s : strings) {
            out.write<uint16_t>(static_cast<uint16_t>(s.size()));
            out.write(s.data(), s.size());
            if (s.size() & 1)
                out.skip(1);
        }
        std::cout << "Done!" << std::endl;
    }
    void write_audio()
    {
        std::cout << "Writing audio.......";
        out.seek(audio_table_offset);
        auto audio_off = audio_offset;
        for (auto& a : audios) {
            out.write<uint64_t>(audio_off);
            audio_off += a.length;
        }
        out.seek(audio_offset);
        for (auto& a : audios)
            out.write(in.base + a.data, a.length);
        std::cout << "Done!" << std::endl;
    }
    void write_bitmaps()
    {
        std::cout << "Writing bitmaps.....";
        out.seek(bitmap_table_offset);
        std::ofstream file(nxfilename, std::ios::app | std::ios::binary);
        std::vector<uint8_t> input;
        std::vector<uint8_t> output;
        for (auto index = 0u; index < bitmaps.size(); ++index) {
            auto& b = bitmaps[index];
            out.write<uint64_t>(bitmap_offset);
            in.seek(b.data);
            auto width = in.read_cint();
            auto height = in.read_cint();
            if (width < 0 || height < 0) {
                std::cerr << "Invalid image size: " << std::dec << width << ", "
                          << height << std::endl;
                throw std::runtime_error { "fak" };
            }
            auto f1 = in.read_cint();
            auto f2 = static_cast<unsigned>(
                in.read<uint8_t>()); // Cast away from char to preserve sanity
            auto n1 = in.read<uint32_t>();
            if (n1) {
                std::cerr << "non-zero n1: "
                          << "0x" << std::setfill('0') << std::setw(8) << std::hex
                          << n1;
                throw std::runtime_error { "fak" };
            }
            auto length = in.read<uint32_t>();
            auto n2 = static_cast<unsigned>(in.read<uint8_t>());
            if (n2) {
                std::cerr << "non-zero n2: "
                          << " 0x" << std::setfill('0') << std::setw(2) << std::hex
                          << n2 << std::endl;
                throw std::runtime_error { "fak" };
            }
            auto size = width * height * 4;
            auto biggest = std::max(static_cast<uint32_t>(size), length);
            input.resize(biggest);
            output.resize(biggest);
            auto original = reinterpret_cast<uint8_t const*>(in.offset);
            auto key = b.key;
            auto decompressed = 0;
            auto decompress = [&] {
                z_stream strm = {};
                strm.next_in = input.data();
                strm.avail_in = length;
                inflateInit(&strm);
                strm.next_out = output.data();
                strm.avail_out = static_cast<unsigned>(output.size());
                auto err = inflate(&strm, Z_FINISH);
                if (err != Z_BUF_ERROR) {
                    if (err != Z_DATA_ERROR) {
                        std::cerr << "zlib error of " << std::dec << err << std::endl;
                    }
                    return false;
                }
                decompressed = static_cast<int>(strm.total_out);
                inflateEnd(&strm);
                return true;
            };
            auto decrypt = [&] {
                auto p = 0u;
                for (auto i = 0u; i <= length - 4;) {
                    auto blen = *reinterpret_cast<uint32_t const*>(original + i);
                    i += 4;
                    if (i + blen > length)
                        return false;
                    for (auto j = 0u; j < blen; ++j)
                        input[p + j] = static_cast<uint8_t>(original[i + j] ^ key[j]);
                    i += blen;
                    p += blen;
                }
                length = p;
                return true;
            };
            std::copy(original, original + length, input.begin());
            if (!decompress() && (!decrypt() || !decompress())) {
                std::cerr << "Unable to inflate: 0x" << std::setfill('0')
                          << std::setw(2) << std::hex << (unsigned)original[0] << " 0x"
                          << std::setfill('0') << std::setw(2) << std::hex
                          << static_cast<unsigned>(original[1]) << std::endl;
                // Just fill the image with blank data so nothing breaks
                f1 = 2;
                f2 = 0;
                decompressed = size;
                std::fill(output.begin(), output.begin() + size, '\0');
            }
            input.swap(output);
            struct color4444 {
                uint8_t b : 4;
                uint8_t g : 4;
                uint8_t r : 4;
                uint8_t a : 4;
            };
            static_assert(sizeof(color4444) == 2, "Your bitpacking sucks");
            struct color8888 {
                uint8_t b;
                uint8_t g;
                uint8_t r;
                uint8_t a;
            };
            static_assert(sizeof(color8888) == 4, "Your bitpacking sucks");
            struct color565 {
                uint16_t b : 5;
                uint16_t g : 6;
                uint16_t r : 5;
            };
            static_assert(sizeof(color565) == 2, "Your bitpacking sucks");
            auto pixels4444 = reinterpret_cast<color4444*>(input.data());
            auto pixels565 = reinterpret_cast<color565*>(input.data());
            auto pixelsout = reinterpret_cast<color8888*>(output.data());
            // Sanity check the sizes
            auto check = decompressed;
            switch (f1) {
            case 1:
                check *= 2;
                break;
            case 2:
                break;
            case 257:
                check *= 2;
                break; // Not sure if this is accurate
            case 513:
                check *= 2;
                break;
            case 1026:
                check *= 4;
                break;
            case 2050:
                check *= 4;
                break;
            default:
                std::cerr << "Unknown image format1 of" << std::dec << f1 << std::endl;
                throw std::runtime_error("Unknown image type!");
            }
            auto pixels = width * height;
            switch (f2) {
            case 0:
                break;
            case 4:
                pixels /= 256;
                break;
            default:
                std::cerr << "Unknown image format2 of" << std::dec
                          << static_cast<unsigned>(f2) << std::endl;
                throw std::runtime_error("Unknown image type!");
            }
            if (check != pixels * 4) {
                std::cerr << "Size mismatch: " << std::dec << width << "," << height
                          << "," << decompressed << "," << f1 << "," << f2 << std::endl;
                throw std::runtime_error("halp!");
            }
            switch (f1) {
            case 1:
                for (auto i = 0; i < pixels; ++i) {
                    auto pixel = pixels4444[i];
                    // pre-select table4
                    auto lookup_table = color_lookup_tables[0];
                    pixelsout[i] = { lookup_table[pixel.b], lookup_table[pixel.g],
                        lookup_table[pixel.r], lookup_table[pixel.a] };
                }
                input.swap(output);
                break;
            case 2:
                // Do nothing
                break;
            case 513:
                for (auto i = 0; i < pixels; ++i) {
                    auto pixel = pixels565[i];

                    pixelsout[i] = { color_lookup_tables[1][pixel.b],
                        color_lookup_tables[2][pixel.g],
                        color_lookup_tables[1][pixel.r], 255 };
                }
                input.swap(output);
                break;
            case 1026:
                squish::DecompressImage(output.data(), width, height, input.data(),
                    squish::kDxt3);
                input.swap(output);
                break;
            case 2050:
                squish::DecompressImage(output.data(), width, height, input.data(),
                    squish::kDxt5);
                input.swap(output);
                break;
            }
            switch (f2) {
            case 0:
                // Do nothing
                break;
            case 4:
                std::cerr << "Format2 of 4 at " << std::dec << index << std::endl;
                scale<16>(input, output, width, height);
                input.swap(output);
                break;
            }
            output.resize(static_cast<size_t>(LZ4_compressBound(size)));
            uint32_t final_size;
            if (hc) {
                final_size = static_cast<uint32_t>(
                    LZ4_compress_HC(reinterpret_cast<char const*>(input.data()),
                        reinterpret_cast<char*>(output.data()), size,
                        LZ4_compressBound(size), 0));
            } else {
                final_size = static_cast<uint32_t>(
                    LZ4_compress_default(reinterpret_cast<char const*>(input.data()),
                        reinterpret_cast<char*>(output.data()), size,
                        LZ4_compressBound(size)));
            }
            bitmap_offset += final_size + 4;
            file.write(reinterpret_cast<char const*>(&final_size), 4);
            file.write(reinterpret_cast<char const*>(output.data()), final_size);
        }
        std::cout << "Done!" << std::endl;
    }
    wztonx(std::filesystem::path filename, bool client, bool hc)
        : client(client)
        , hc(hc)
    {
        wzfilename = filename.native();
        nxfilename = filename.replace_extension(".nx").native();
        if (!std::ifstream { wzfilename }.is_open()) {
            return;
        }
        std::cout << wzfilename << " -> " << nxfilename << std::endl;
    }
    void convert_file()
    {
        parse_file();
        open_output();
        write_nodes();
        write_strings();
        if (client) {
            write_audio();
            write_bitmaps();
        }
    }
};
struct imgtonx : wztonx {
    imgtonx(std::filesystem::path filename, bool client, bool hc)
        : wztonx { filename, client, hc }
    {
    }
    void parse_file() override
    {
        std::cout << "Parsing input.......";
        in.open(wzfilename);
        add_string({});
        img(0, 0);
        finish_parse();
    }
};
} // namespace nl
