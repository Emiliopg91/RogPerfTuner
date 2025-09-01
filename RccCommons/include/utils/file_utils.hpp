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
        return exists(std::filesystem::path(path));
    }
    static inline bool exists(const std::filesystem::path &path)
    {
        return std::filesystem::exists(path);
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

    inline static std::string dirname(std::string file)
    {
        std::string pathArg = file;
        size_t pos = pathArg.find_last_of('/');
        return (pos != std::string::npos) ? pathArg.substr(0, pos) : ".";
    }

    inline static void mkdirs(std::string path)
    {
        std::filesystem::path dirPath(path);
        mkdirs(dirPath);
    }

    inline static void mkdirs(std::filesystem::path dirPath)
    {
        if (!std::filesystem::exists(dirPath))
        {
            std::filesystem::create_directories(dirPath);
        }
    }

    inline static std::filesystem::file_time_type getMTime(std::string path)
    {
        return getMTime(std::filesystem::path(path));
    }

    inline static std::filesystem::file_time_type getMTime(std::filesystem::path path)
    {
        return std::filesystem::last_write_time(path);
    }

    inline static void move(std::string src, std::string dst)
    {
        move(std::filesystem::path(src), std::filesystem::path(dst));
    }

    inline static void move(std::filesystem::path src, std::filesystem::path dst)
    {
        std::filesystem::rename(src, dst);
    }

    inline static void copy(std::string src, std::string dst)
    {
        copy(std::filesystem::path(src), std::filesystem::path(dst));
    }

    inline static void copy(std::filesystem::path src, std::filesystem::path dst)
    {
        std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    }
};