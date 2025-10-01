#include "../../include/utils/constants.hpp"

#include <limits.h>
#include <unistd.h>

const std::string Constants::APP_NAME = "RogControlCenter";

const std::string Constants::APP_VERSION = "4.3.12";

const pid_t Constants::PID = getpid();

const std::string Constants::PLUGIN_VERSION = "1.2.3";

const bool Constants::DEV_MODE = true;

const std::string Constants::HOME_DIR = std::getenv("HOME");

const std::string Constants::APPIMAGE_FILE = []() {
	const char* appimage = std::getenv("APPIMAGE");
	if (appimage == nullptr) {
		appimage = "";
	}
	return appimage;
}();

const std::string Constants::ASSETS_DIR = [] {
	const char* dir = std::getenv("RCC_ASSETS_DIR");
	if (!dir) {
		dir = "/usr/share/rogcontrolcenter";
	}
	return std::string(dir);
}();

const std::string Constants::WRAPPER_PATH = [] {
	return (APPIMAGE_FILE.empty() ? ASSETS_DIR : (HOME_DIR + "/.RogControlCenter")) + "/bin/steam/run";
}();

const std::string Constants::LOG_FILE_NAME		  = APP_NAME;
const std::string Constants::LOG_RUNNER_FILE_NAME = "Runner";
const std::string Constants::LOG_ORGB_FILE_NAME	  = "OpenRGB";

const std::string Constants::USR_SHARE_OCL_DIR = "/etc/OpenCL/vendors/";
const std::string Constants::UDEV_RULES		   = "/usr/lib/udev/rules.d/60-openrgb.rules";
const std::string Constants::USR_SHARE_VK_DIR  = "/usr/share/vulkan/icd.d/";
const std::string Constants::TMP_UDEV_PATH	   = "/tmp/60-openrgb.rules";
const std::string Constants::LOCK_FILE		   = "/tmp/RogControlCenter.lock";
const std::string Constants::SOCKET_FILE	   = "/tmp/RogControlCenter.sock";

const std::string Constants::ASSETS_BIN_DIR	   = ASSETS_DIR + "/bin";
const std::string Constants::TRANSLATIONS_FILE = ASSETS_DIR + "/translations.json";
const std::string Constants::ASSET_ICONS_DIR   = ASSETS_DIR + "/icons";
const std::string Constants::ASSET_ICON_FILE   = ASSETS_DIR + "/icons/icon.svg";
const std::string Constants::ORGB_DIR		   = ASSETS_DIR + "/OpenRGB";
const std::string Constants::ORGB_UDEV_PATH	   = ASSETS_DIR + "/OpenRGB/usr/lib/udev/rules.d/60-openrgb.rules";
const std::string Constants::ORGB_PATH		   = ASSETS_DIR + "/OpenRGB/OpenRGB.sh";
const std::string Constants::RCCDC_ASSET_PATH  = ASSETS_DIR + "/RccDeckyCompanion";

const std::string Constants::AUTOSTART_FILE		 = HOME_DIR + "/.config/autostart/RogControlCenter.desktop";
const std::string Constants::APP_DRAW_FILE		 = HOME_DIR + "/.local/share/applications/RogControlCenter.desktop";
const std::string Constants::BIN_DIR			 = HOME_DIR + "/.RogControlCenter/bin";
const std::string Constants::BIN_APPLICATION_DIR = HOME_DIR + "/.RogControlCenter/bin/application";
const std::string Constants::LAUNCHER_FILE		 = HOME_DIR + "/.RogControlCenter/bin/application/launcher.sh";
const std::string Constants::CONFIG_DIR			 = HOME_DIR + "/.RogControlCenter/config";
const std::string Constants::CONFIG_FILE		 = HOME_DIR + "/.RogControlCenter/config/config.json";
const std::string Constants::ICONS_DIR			 = HOME_DIR + "/.RogControlCenter/icons";
const std::string Constants::ICON_FILE			 = HOME_DIR + "/.RogControlCenter/icons/icon.svg";
const std::string Constants::ICON_45_FILE		 = HOME_DIR + "/.RogControlCenter/icons/icon-45x45.png";
const std::string Constants::LIB_DIR			 = HOME_DIR + "/.RogControlCenter/lib";
const std::string Constants::LIB_VK_DIR			 = HOME_DIR + "/.RogControlCenter/lib/vk/icd.d/";
const std::string Constants::LIB_OCL_DIR		 = HOME_DIR + "/.RogControlCenter/lib/ocl/icd.d/";
const std::string Constants::LOG_DIR			 = HOME_DIR + "/.RogControlCenter/logs";
const std::string Constants::LOG_OLD_DIR		 = HOME_DIR + "/.RogControlCenter/logs/old";
const std::string Constants::USER_PLUGIN_DIR	 = HOME_DIR + "/.RogControlCenter/plugin";
const std::string Constants::UPDATE_DIR			 = HOME_DIR + "/.RogControlCenter/update";
const std::string Constants::UPDATE_FILE		 = HOME_DIR + "/.RogControlCenter/update/RogControlCenter.AppImage";
const std::string Constants::UPDATE_TMP_FILE	 = HOME_DIR + "/.RogControlCenter/update/RogControlCenter.AppImage.tmp";
const std::string Constants::RCDCC_SOCKET_PATH	 = HOME_DIR + "/homebrew/data/RCCDeckyCompanion/socket";
const std::string Constants::PLUGINS_FOLDER		 = HOME_DIR + "/homebrew/plugins";
const std::string Constants::RCCDC_PATH			 = HOME_DIR + "/homebrew/plugins/RCCDeckyCompanion";
const std::string Constants::RCCDC_PACKAGE_FILE	 = HOME_DIR + "/homebrew/plugins/RCCDeckyCompanion/package.json";
const std::string Constants::DECKY_SERVICE_PATH	 = HOME_DIR + "/homebrew/services/PluginLoader";

const std::string Constants::GAME_CFG	= "gameConfig";
const std::string Constants::PERF_PROF	= "nexPerformanceProfile";
const std::string Constants::DEC_BRIGHT = "decRgbBrightness";
const std::string Constants::INC_BRIGHT = "incRgbBrightness";
const std::string Constants::NEXT_EFF	= "nextRgbEffect";

const int Constants::WS_PORT = 18158;

const std::vector<std::string> Constants::RCCDC_REQUIRED_PIP = {"dataclasses-json", "asyncio"};

const std::string Constants::FLATPAK_MANGOHUD		   = "org.freedesktop.Platform.VulkanLayer.MangoHud/x86_64/24.08";
const std::string Constants::FLATPAK_MANGOHUD_OVERRIDE = "--filesystem=xdg-config/MangoHud:ro";

const std::string Constants::LOGGER_PATTERN = "[%Y-%m-%d %H:%M:%S.%e][%-7l][%n] %v";