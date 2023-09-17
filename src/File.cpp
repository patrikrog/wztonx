#include "File.h"
#include <filesystem>
#include <iostream>

File::File(std::string filename)
{

    m_file = fopen(filename.c_str(), "r");
    struct stat st;
    stat(filename.c_str(), &st);
    m_size = st.st_size;

    std::cerr << m_size << std::endl;
}

File::~File()
{
}
