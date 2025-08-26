#pragma once

#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <filesystem>
#include "string_utils.hpp"

class Constants
{
private:
    static std::string getExecutablePath()
    {
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            return std::string(buffer);
        }
        throw std::runtime_error("No se pudo obtener la ruta del ejecutable");
    }
    static std::string getExecutableDir()
    {
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            std::filesystem::path exePath(buffer);
            return exePath.parent_path().string(); // Solo el directorio
        }
        throw std::runtime_error("No se pudo obtener la ruta del ejecutable");
    }

public:
    inline static const std::string ASSETS_DIR = []
    {
        const char *dir = std::getenv("RCC_ASSETS_DIR");
        if (!dir)
        {
            dir = std::getenv("PWD");
        }
        return std::string(dir);
    }();

    inline static const bool DEV_MODE = std::getenv("RCC_MODE") == nullptr || StringUtils::toLowerCase(std::getenv("RCC_MODE")) == "dev";

    inline static const std::string EXECUTABLE_PATH = Constants::getExecutablePath();
    inline static const std::string EXECUTABLE_DIR = Constants::getExecutableDir();

    inline static const std::string HOME_DIR = std::getenv("HOME");
    inline static const std::string APP_DIR = Constants::HOME_DIR + "/.config/RogControlCenter";

    inline static const std::string CONFIG_DIR = Constants::APP_DIR + "/config";
    inline static const std::string CONFIG_FILE = Constants::CONFIG_DIR + "/config.json";

    inline static const std::string LOG_DIR = Constants::APP_DIR + "/logs";
    inline static const std::string LOG_OLD_DIR = Constants::LOG_DIR + "/old";
    inline static const std::string LOG_FILE = Constants::LOG_DIR + "/current.log";
    inline static const std::string LOG_ORGB_FILE = Constants::LOG_DIR + "/OpenRGB.log";

    inline static const std::string ICONS_DIR = Constants::ASSETS_DIR + "/icons";
    inline static const std::string ICON_FILE = Constants::ICONS_DIR + "/icon-45x45.png";

    inline static const std::string ORGB_DIR = Constants::ASSETS_DIR + "/OpenRGB";
    inline static const std::string ORGB_RULES_FILE = Constants::ORGB_DIR + "/usr/lib/udev/rules.d/60-openrgb.rules";
    inline static const std::string ORGB_PATH = Constants::ORGB_DIR + "/AppRun";

    inline static const std::string URL_PERF_PROF = "/performance/profile/next";
    inline static const std::string URL_INC_BRIGHT = "/rgb/brightness/increase";
    inline static const std::string URL_DEC_BRIGHT = "/rgb/brightness/decrease";
    inline static const std::string URL_NEXT_EFF = "/rgb/effect/next";

    inline static const int HTTP_PORT = 18157;
};
