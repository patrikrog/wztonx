#include "File.h"
#include <filesystem>
#include <iostream>

File::File(std::filesystem::path filename)
{
    m_file.open(filename.c_str(), std::ios::in | std::ios::binary);
    m_size = std::filesystem::file_size(filename);

    std::cerr << m_size << std::endl;
}

File::~File()
{
}
