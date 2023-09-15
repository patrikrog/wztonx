#ifndef CONVERTER_H_
#define CONVERTER_H_

#include "File.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sys/types.h>

namespace nl {
class Converter {
    typedef key_t uint8_t;

public:
    Converter(bool client_flag, bool high_compression_flag);
    ~Converter();

    void convert(std::filesystem::path);

private:
    void convert_img(std::filesystem::path) { std::cout << "TODO" << std::endl; }
    void convert_wz(std::filesystem::path);

    bool m_client_flag = false;
    bool m_high_compression_flag = false;

    key_t* m_gms_key;
    key_t* m_kms_key;
};
} // namespace nl

#endif // CONVERTER_H_
