#ifndef CONVERTER_H_
#define CONVERTER_H_
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>

#include "File.h"
#include "Utils.h"

#define MAGIC_KEY_LENGTH 65536

class Converter {
    struct Key {
        int length = 0;
        key_t* data = new key_t[MAGIC_KEY_LENGTH];

        void load(uint8_t* input)
        {
            std::copy(input, input + MAGIC_KEY_LENGTH, data);
        }
    };

public:
    Converter(Utils::Flags);
    ~Converter();
    void convert(std::string);

private:
    void convert_img(File);
    void convert_wz(File);

    void load_file(std::string);

    bool m_client_flag = false;
    bool m_high_compression_flag = false;
    bool m_server_flag = false;

    File* m_file;

    Key m_gms_key = *new Key { MAGIC_KEY_LENGTH };
    Key m_kms_key = *new Key { MAGIC_KEY_LENGTH };
};

#endif // CONVERTER_H_
