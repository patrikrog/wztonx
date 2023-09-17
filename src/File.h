#ifndef FILE_H_
#define FILE_H_

#include "Utils.h"

#include <cstring>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

class File {
public:
    File(std::string);
    ~File();

    void operator=(const File other)
    {
        memcpy((void*)this, (void*)&other, other.size());
    }

    FILE* data() { return m_file; }

    size_t size() { return m_size; }
    const size_t size() const { return m_size; }

private:
    FILE* m_file = nullptr;
    std::string m_filename;

    size_t m_size { 0 };
};
#endif // FILE_H_
