#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

class FileUtils
{
private:
public:
    static inline bool exists(const std::string &path)
    {
        return std::filesystem::exists(std::filesystem::path(path));
    }

    static inline void createDirectory(const std::string &path)
    {
        std::filesystem::path dirPath(path);

        if (!std::filesystem::exists(dirPath))
        {
            if (!std::filesystem::create_directories(dirPath))
            {
                throw std::runtime_error("Couldn't create directory: " + path);
            }
        }
    }

    static inline void copy(const std::string src, const std::string dst)
    {
        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
    }

    static inline void writeFileContent(const std::string path, const std::string content)
    {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open())
        {
            throw new std::runtime_error("Couldn't open file " + path);
        }

        file << content;
        file.close();
    }

    static inline std::string readFileContent(const std::string &path)
    {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open())
        {
            throw new std::runtime_error("Couldn't open file " + path);
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        file.close();

        return ss.str();
    }
};