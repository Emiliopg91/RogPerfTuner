#pragma once

#include <string>
#include <filesystem>
#include <iostream>

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
};