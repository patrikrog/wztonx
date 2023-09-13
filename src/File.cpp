#include "File.h"

File::File(std::filesystem::path file)
{
    m_file.open(file.c_str(), std::ios::in | std::ios::binary);
}
