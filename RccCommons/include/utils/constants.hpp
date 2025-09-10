#pragma once

#include <limits.h>
#include <unistd.h>

#include <string>

class Constants {
  public:
	inline static const std::string APP_NAME = "RogControlCenter";

	inline const static std::string APP_VERSION = "4.0.10";

	inline const static std::string PLUGIN_VERSION = "1.2.1 ";

	inline const static bool DEV_MODE = true;

	inline static const std::string HOME_DIR = std::getenv("HOME");

	inline static const std::string APPIMAGE_FILE = []() {
		const char* appimage = std::getenv("APPIMAGE");
		if (appimage == nullptr)
			appimage = "";
		return appimage;
	}();

	inline static const std::string ASSETS_DIR = [] {
		const char* dir = std::getenv("RCC_ASSETS_DIR");
		if (!dir) {
			dir = std::getenv("PWD");
		}
		return std::string(dir);
	}();

	inline static const std::string USR_SHARE_OCL_DIR = "/etc/OpenCL/vendors/";
	inline static const std::string LOCK_FILE		  = "/tmp/RogControlCenter.lock";
	inline static const std::string UDEV_RULES		  = "/usr/lib/udev/rules.d/60-openrgb.rules";
	inline static const std::string USR_SHARE_VK_DIR  = "/usr/share/vulkan/icd.d/";
	inline static const std::string TMP_UDEV_PATH	  = "/tmp/60-openrgb.rules";

	inline static const std::string ASSETS_BIN_DIR	 = ASSETS_DIR + "/bin";
	inline static const std::string ASSET_ICONS_DIR	 = ASSETS_DIR + "/icons";
	inline static const std::string ASSET_ICON_FILE	 = ASSETS_DIR + "/icons/icon.svg";
	inline static const std::string ORGB_DIR		 = ASSETS_DIR + "/OpenRGB";
	inline static const std::string ORGB_UDEV_PATH	 = ASSETS_DIR + "/OpenRGB/usr/lib/udev/rules.d/60-openrgb.rules";
	inline static const std::string ORGB_PATH		 = ASSETS_DIR + "/OpenRGB/AppRun";
	inline static const std::string RCCDC_ASSET_PATH = ASSETS_DIR + "/RccDeckyCompanion";

	inline static const std::string AUTOSTART_FILE		= HOME_DIR + "/.config/autostart/RogControlCenter.desktop";
	inline static const std::string APP_DRAW_FILE		= HOME_DIR + "/.local/share/applications/RogControlCenter.desktop";
	inline static const std::string BIN_DIR				= HOME_DIR + "/.RogControlCenter/bin";
	inline static const std::string BIN_APPLICATION_DIR = HOME_DIR + "/.RogControlCenter/bin/application";
	inline static const std::string LAUNCHER_FILE		= HOME_DIR + "/.RogControlCenter/bin/application/launcher.sh";
	inline static const std::string CONFIG_DIR			= HOME_DIR + "/.RogControlCenter/config";
	inline static const std::string CONFIG_FILE			= HOME_DIR + "/.RogControlCenter/config/config.json";
	inline static const std::string ICONS_DIR			= HOME_DIR + "/.RogControlCenter/icons";
	inline static const std::string ICON_FILE			= HOME_DIR + "/.RogControlCenter/icons/icon.svg";
	inline static const std::string ICON_45_FILE		= HOME_DIR + "/.RogControlCenter/icons/icon-45x45.png";
	inline static const std::string LIB_DIR				= HOME_DIR + "/.RogControlCenter/lib";
	inline static const std::string LIB_VK_DIR			= HOME_DIR + "/.RogControlCenter/lib/vk/icd.d/";
	inline static const std::string LIB_OCL_DIR			= HOME_DIR + "/.RogControlCenter/lib/ocl/icd.d/";
	inline static const std::string LOG_DIR				= HOME_DIR + "/.RogControlCenter/logs";
	inline static const std::string LOG_OLD_DIR			= HOME_DIR + "/.RogControlCenter/logs/old";
	inline static const std::string LOG_ORGB_FILE		= HOME_DIR + "/.RogControlCenter/logs/OpenRGB.log";
	inline static const std::string USER_PLUGIN_DIR		= HOME_DIR + "/.RogControlCenter/plugin";
	inline static const std::string UPDATE_DIR			= HOME_DIR + "/.RogControlCenter/update";
	inline static const std::string UPDATE_FILE			= HOME_DIR + "/.RogControlCenter/update/RogControlCenter.AppImage";
	inline static const std::string UPDATE_TMP_FILE		= HOME_DIR + "/.RogControlCenter/update/RogControlCenter.AppImage.tmp";
	inline static const std::string PLUGINS_FOLDER		= HOME_DIR + "/homebrew/plugins";
	inline static const std::string RCCDC_PATH			= HOME_DIR + "/homebrew/plugins/RCCDeckyCompanion";
	inline static const std::string DECKY_SERVICE_PATH	= HOME_DIR + "/homebrew/services/PluginLoader";

	inline static const std::string LOG_FILE_NAME		 = "RogControlCenter";
	inline static const std::string LOG_RUNNER_FILE_NAME = "Runner";

	inline static const std::string URL_PERF_PROF  = "/performance/profile/next";
	inline static const std::string URL_INC_BRIGHT = "/rgb/brightness/increase";
	inline static const std::string URL_DEC_BRIGHT = "/rgb/brightness/decrease";
	inline static const std::string URL_NEXT_EFF   = "/rgb/effect/next";
	inline static const std::string URL_RENICE	   = "/performance/renice";
	inline static const std::string URL_GAME_CFG   = "/game/config";

	inline static const int HTTP_PORT = 18157;
	inline static const int WS_PORT	  = HTTP_PORT + 1;
};
