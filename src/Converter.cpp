#include "Converter.h"
#include "old/Keys.h"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sys/types.h>

Converter::Converter(Utils::Flags flag)
    : m_client_flag(flag.client)
    , m_high_compression_flag(flag.high_compression)
    , m_server_flag(flag.server)
{
    m_gms_key.load(key_gms);
    m_kms_key.load(key_kms);
}
Converter::~Converter()
{
}

void Converter::load_file(std::string filename)
{
    File file = File(filename);
    m_file = &file;
}

void Converter::convert(std::string filename)
{
    std::filesystem::path path = filename;
    this->load_file(filename);
    if (path.extension() == ".img") {
        std::cout << "This is a .img file." << std::endl;
        this->convert_img(filename);
    } else if (path.extension() == ".wz") {
        std::cout << "This is a .wz file." << std::endl;
        this->convert_wz(filename);
    }
}

void Converter::convert_wz(File file)
{
}

void Converter::convert_img(File file)
{
}
