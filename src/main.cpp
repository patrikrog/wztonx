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

#include "src/wztonx.h"

int main(int argc, char* argv[])
{
    std::cout << "WzToNx Converter";
    std::cout
        << "Copyright (C) 2014-2023 Peter Atashian, Ryan Payton, Patrik Rogalski";
    std::cout << "Licensed under GNU Affero General Public License";
    std::cout << "Converts WZ files into NX files";

    auto client_flag = false;
    auto server_flag = false;
    auto high_compression = false;

    std::vector<std::filesystem::path> files;

    while (int arguments = getopt(argc, argv, "csh") != -1) {
        switch (arguments) {
        case 's':
            server_flag = true;
            break;
        case 'h':
            high_compression = true;
            break;
        default:
        case 'c':
            client_flag = true;
            break;
        }
    }

    auto convert_file = [&](std::filesystem::path const& file) {
        auto extension = file.extension().native();
        auto converter = nl::converter();

        if (extension == ".img") {
            converter.convert_imge(file, client_flag, high_compression);

            // nl::imgtonx { file, client_flag, high_compression }.convert_file();
        } else if (extension == ".wz") {
            converter.convert_wz(file, client_flag, high_compression);

            // nl::wztonx { file, client_flag, high_compression }.convert_file();
        }
    };
    for (auto& path : files) {
        if (!std::filesystem::is_regular_file(path)) {
            std::cerr << "Not a regular file?: " << path;
            exit(1);
        }
        convert_file(path);
    }

    return 0;
}
