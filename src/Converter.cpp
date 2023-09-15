#include "Converter.h"
#include "Keys.h"
#include <cstring>
#include <iostream>
#include <sys/types.h>

namespace nl {

Converter::Converter(bool client_flag, bool high_compression_flag)
    : m_client_flag(client_flag)
    , m_high_compression_flag(high_compression_flag)
{
    // allocating for twice the size of the key, for some reason
    m_gms_key = new key_t[131072];
    m_kms_key = new key_t[131072];

    // copy in the key from Keys.h since I'm too lazy to figure anything else out for now.
    std::copy(std::begin(key_gms), std::end(key_gms), m_gms_key);
    std::copy(std::begin(key_kms), std::end(key_kms), m_kms_key);
}
Converter::~Converter()
{
}

void Converter::convert(std::filesystem::path filename)
{
    if (filename.extension() == ".img") {
        std::cout << "This is a .img file." << std::endl;
        this->convert_img(filename);
    } else if (filename.extension() == ".wz") {
        std::cout << "This is a .wz file." << std::endl;
        this->convert_wz(filename);
    }
}
void Converter::convert_wz(std::filesystem::path filename)
{
    File file = File(filename);
}
} // namespace nl
