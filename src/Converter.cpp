#include "Converter.h"
#include <iostream>

namespace nl {

Converter::Converter(bool client_flag, bool high_compression_flag)
    : m_client_flag(client_flag)
    , m_high_compression_flag(high_compression_flag)
{
}
Converter::~Converter()
{
}

void Converter::convert(std::filesystem::path filename)
{
    auto file { filename };
    if (file.extension() == ".img") {
        std::cout << "This is a .img file." << std::endl;
        this->convert_img(file);
    } else if (file.extension() == ".wz") {
        std::cout << "This is a .wz file." << std::endl;
        this->convert_wz(file);
    }
}
} // namespace nl
