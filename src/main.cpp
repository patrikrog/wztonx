/*
  This file is part of the continued NoLifeStory project
  Copyright (C) 2014-2023  Peter Atashian, Ryan Payton, Patrik Rogalski


  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.


  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.


  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <filesystem>
#include <iostream>
#include <regex>
#include <unistd.h>
#include <vector>

#include "Converter.h"

int main(int argc, char* argv[])
{
    std::cout << "WzToNx Converter" << std::endl;
    std::cout
        << "Copyright (C) 2014-2023 Peter Atashian, Ryan Payton, Patrik Rogalski" << std::endl;
    std::cout << "Licensed under GNU Affero General Public License" << std::endl;
    std::cout << "Converts WZ files into NX files" << std::endl;

    auto client_flag = false;
    auto server_flag = false;
    auto high_compression_flag = false;

    std::vector<std::filesystem::path> files;

    while (int arguments = getopt(argc, argv, "csh") != -1) {
        switch (arguments) {
        case 's':
            server_flag = true;
            break;
        case 'h':
            high_compression_flag = true;
            break;
        default:
        case 'c':
            client_flag = true;
            break;
        }
    }

    for (auto& argument : std::vector<std::string>(argv + 1, argv + argc)) {
        // this is to skip over the command-line flags
        if (argument[0] == '-')
            continue;
        files.emplace_back(argument);
    }
    auto converter = nl::Converter(client_flag, high_compression_flag);

    for (auto& path : files) {
        if (!std::filesystem::is_regular_file(path)) {
            std::cerr << "Not a regular file?: " << path;
            exit(1);
        }
        std::cout << path << std::endl;
        converter.convert(path);
    }

    return 0;
}
