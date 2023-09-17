#include <filesystem>
#include <iostream>
#include <regex>
#include <unistd.h>
#include <vector>

#include "Converter.h"
#include "Utils.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "WzToNx Converter" << std::endl;
        std::cout << "Converts WZ files into NX files" << std::endl;
        std::cout << "Usage: " << argv[0] << " [-csh] [input.wz]" << std::endl;
    }

    std::vector<std::filesystem::path> files;
    Utils::Flags flags;

    while (int arguments = getopt(argc, argv, "csh") != -1) {
        switch (arguments) {
        case 's':
            flags.server = true;
            break;
        case 'h':
            flags.high_compression = true;
            break;
        default:
        case 'c':
            flags.client = true;
            break;
        }
    }

    // this is to skip over the command-line flags
    for (auto& argument : std::vector<std::string>(argv + 1, argv + argc)) {
        if (argument[0] == '-')
            continue;
        files.emplace_back(argument);
    }

    auto converter = Converter { flags };

    for (auto& path : files) {
        if (!std::filesystem::is_regular_file(path)) {
            std::cerr << "Not a regular file?: " << path;
            exit(1);
        }
        converter.convert(path);
    }

    return 0;
}
