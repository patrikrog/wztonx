#ifndef CONVERTER_H_
#define CONVERTER_H_

#include "File.h"
#include <filesystem>

namespace nl {
class Converter {
public:
    Converter(bool client_flag, bool high_compression_flag);
    ~Converter();

    void convert(std::filesystem::path);

private:
    std::string convert_img(std::filesystem::path) { return "TODO"; }
    std::string convert_wz(std::filesystem::path) { return "TODO"; }

    bool m_client_flag = false;
    bool m_high_compression_flag = false;
};
} // namespace nl

#endif // CONVERTER_H_
