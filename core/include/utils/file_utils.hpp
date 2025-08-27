#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "../logger/logger.hpp"

class FileUtils
{
private:
    inline static Logger logger{"FileUtils"};

public:
    static inline bool exists(const std::string &path)
    {
        return std::filesystem::exists(std::filesystem::path(path));
    }

    static inline bool createDirectory(const std::string &path)
    {
        std::filesystem::path dirPath(path);

        try
        {
            if (!std::filesystem::exists(dirPath))
            {
                if (std::filesystem::create_directories(dirPath))
                {
                    logger.info(fmt::format("Directorio creado: {}", path));
                    return true;
                }
                else
                {
                    logger.error(fmt::format("No se pudo crear el directorio: {}", path));
                    return false;
                }
            }
            // El directorio ya existía
            return true;
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            logger.error(fmt::format("Error de filesystem: {}", e.what()));
            return false;
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
            logger.error("Couldn't open file " + path);
            return;
        }

        file << content;
        file.close();
    }
};