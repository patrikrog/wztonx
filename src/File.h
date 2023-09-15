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
#ifndef FILE_H_
#define FILE_H_

#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

class File {
public:
    File(std::filesystem::path);
    ~File();

    std::string extension() { return m_filename.extension(); }

private:
    std::fstream m_file;
    std::filesystem::path m_filename;

    size_t m_size = 0;
};
#endif // FILE_H_
