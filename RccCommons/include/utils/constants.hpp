#pragma once

#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <filesystem>
#include "file_utils.hpp"
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

    inline const static std::string APP_NAME = "RogControlCenter";
    inline const static std::string APP_VERSION = []()
    {
        if (FileUtils::exists(ASSETS_DIR + "/" + "version"))
            return StringUtils::trim(FileUtils::readFileContent(ASSETS_DIR + "/" + "version"));
        return std::string("");
    }();

    inline static const std::string LOCK_FILE = "/tmp/" + APP_NAME + ".lock";
    ;

    inline static const std::string EXECUTABLE_PATH = Constants::getExecutablePath();
    inline static const std::string EXECUTABLE_DIR = Constants::getExecutableDir();

    inline static const std::string HOME_DIR = std::getenv("HOME");

    inline static const std::string DECKY_SERVICE_PATH = HOME_DIR + "/homebrew/services/PluginLoader";

    inline static const std::string APPIMAGE_FILE = []()
    {
        const char *appimage = std::getenv("APPIMAGE");
        if (appimage == nullptr)
            appimage = "";
        return appimage;
    }();
    inline static const std::string AUTOSTART_FILE = HOME_DIR + "/.config/autostart/" + APP_NAME + ".desktop";
    inline static const std::string APP_DRAW_FILE = HOME_DIR + "/.local/share/applications/" + APP_NAME + ".desktop";

    inline static const std::string APP_DIR = Constants::HOME_DIR + "/." + APP_NAME;

    inline static const std::string UPDATE_DIR = Constants::APP_DIR + "/update";
    inline static const std::string UPDATE_FILE = Constants::UPDATE_DIR + "/" + APP_NAME + ".AppImage";

    inline static const std::string ASSETS_BIN_DIR = Constants::ASSETS_DIR + "/bin";
    inline static const std::string BIN_DIR = Constants::APP_DIR + "/bin";
    inline static const std::string BIN_APPLICATION_DIR = Constants::BIN_DIR + "/application";
    inline static const std::string LAUNCHER_FILE = Constants::BIN_APPLICATION_DIR + "/launcher.sh";

    inline static const std::string ICONS_DIR = Constants::APP_DIR + "/icons";
    inline static const std::string ICON_FILE = Constants::ICONS_DIR + "/icon.svg";
    inline static const std::string ICON_45_FILE = Constants::ICONS_DIR + "/icon-45x45.png";

    inline static const std::string CONFIG_DIR = Constants::APP_DIR + "/config";
    inline static const std::string CONFIG_FILE = Constants::CONFIG_DIR + "/config.json";

    inline static const std::string LOG_DIR = Constants::APP_DIR + "/logs";
    inline static const std::string LOG_OLD_DIR = Constants::LOG_DIR + "/old";
    inline static const std::string LOG_FILE_NAME = "RogControlCenter";
    inline static const std::string LOG_ORGB_FILE = Constants::LOG_DIR + "/OpenRGB.log";
    inline static const std::string LOG_RUNNER_FILE_NAME = "Runner";

    inline static const std::string ASSET_ICONS_DIR = Constants::ASSETS_DIR + "/icons";
    inline static const std::string ASSET_ICON_FILE = Constants::ASSET_ICONS_DIR + "/icon.svg";

    inline static const std::string ORGB_DIR = Constants::ASSETS_DIR + "/OpenRGB";
    inline static const std::string ORGB_RULES_FILE = Constants::ORGB_DIR + "/usr/lib/udev/rules.d/60-openrgb.rules";
    inline static const std::string ORGB_PATH = Constants::ORGB_DIR + "/AppRun";

    inline static const std::string URL_PERF_PROF = "/performance/profile/next";
    inline static const std::string URL_INC_BRIGHT = "/rgb/brightness/increase";
    inline static const std::string URL_DEC_BRIGHT = "/rgb/brightness/decrease";
    inline static const std::string URL_NEXT_EFF = "/rgb/effect/next";
    inline static const std::string URL_RENICE = "/performance/renice";
    inline static const std::string URL_GAME_CFG = "/game/config";

    inline static const std::string PLUGINS_FOLDER = HOME_DIR + "/homebrew/plugins";
    inline static const std::string RCCDC_PATH = PLUGINS_FOLDER + "/RCCDeckyCompanion";
    inline static const std::string USER_PLUGIN_DIR = Constants::APP_DIR + "/plugin";
    inline static const std::string RCCDC_ASSET_PATH = ASSETS_DIR + "/RccDeckyCompanion";

    inline static const int HTTP_PORT = 18157;
    inline static const int WS_PORT = HTTP_PORT + 1;
};
