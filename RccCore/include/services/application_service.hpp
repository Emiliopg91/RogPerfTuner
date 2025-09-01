#pragma once

#include <sstream>
#include "RccCommons.hpp"

class ApplicationService
{

public:
    static ApplicationService &getInstance()
    {
        static ApplicationService instance;
        return instance;
    }

    void applyUpdate();

private:
    Logger logger{"ApplicationService"};
    bool rccdcEnabled = false;

    std::string buildDesktopFile()
    {
        std::ostringstream ss;
        ss << "[Desktop Entry]\n"
           << "Exec=" << Constants::LAUNCHER_FILE << "\n"
           << "Icon=" << Constants::ICON_FILE << "\n"
           << "WName=" << Constants::APP_NAME << "\n"
           << "Comment=An utility to manage Asus Rog laptop performance\n"
           << "Path=\n"
           << "Terminal=False\n"
           << "Type=Application\n"
           << "Categories=Utility;\n";
        return ss.str();
    }

    std::string buildLaunchFile()
    {
        std::ostringstream ss;
        ss << "#!/bin/bash\n"
           << "UPDATE_PATH=\"" << Constants::UPDATE_FILE << "\"\n"
           << "APPIMAGE_PATH=\"" << Constants::APPIMAGE_FILE << "\"\n"
           << "\n"
           << "if [[ -f \"$UPDATE_PATH\" ]]; then\n"
           << "  cp \"$UPDATE_PATH\" \"$APPIMAGE_PATH\"\n"
           << "  chmod 755 \"$APPIMAGE_PATH\"\n"
           << "  rm \"$UPDATE_PATH\"\n"
           << "fi\n"
           << Constants::APPIMAGE_FILE << "\n";
        return ss.str();
    }

    ApplicationService();
};
