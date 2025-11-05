#pragma once

#include <sys/types.h>

#include <string>
#include <vector>

class Constants {
  public:
	static const std::string APP_NAME;
	static const std::string EXEC_NAME;

	const static std::string APP_VERSION;

	const static pid_t PID;

	const static std::string PLUGIN_VERSION;

	static const std::string HOME_DIR;

	static const std::string ASSETS_DIR;

	static const std::string USR_SHARE_OCL_DIR;
	static const std::string UDEV_RULES;
	static const std::string USR_SHARE_VK_DIR;
	static const std::string LOCK_FILE;

	static const std::string ASSETS_BIN_DIR;
	static const std::string ASSET_ICONS_DIR;
	static const std::string ASSET_ICON_FILE;
	static const std::string ASSET_ICON_45_FILE;

	static const std::string ORGB_DIR;
	static const std::string ORGB_PATH;
	static const std::string RCCDC_ASSET_PATH;

	static const std::string AUTOSTART_FILE;
	static const std::string APP_DRAW_FILE;
	static const std::string BIN_DIR;
	static const std::string BIN_APPLICATION_DIR;
	static const std::string BIN_PERFORMANCE_DIR;
	static const std::string NEXT_PROFILE_PATH;
	static const std::string BIN_RGB_DIR;
	static const std::string DECREASE_BRIGHTNESS_PATH;
	static const std::string INCREASE_BRIGHTNESS_PATH;
	static const std::string NEXT_EFFECT_PATH;
	static const std::string BIN_STEAM_DIR;
	static const std::string FLATPAK_WRAPPER_PATH;
	static const std::string STEAM_WRAPPER_PATH;
	static const std::string CONFIG_DIR;
	static const std::string CONFIG_FILE;
	static const std::string LOGOS_DIR;
	static const std::string LIB_DIR;
	static const std::string LIB_VK_DIR;
	static const std::string LIB_OCL_DIR;
	static const std::string LOG_DIR;
	static const std::string LOG_OLD_DIR;
	static const std::string USER_PLUGIN_DIR;
	static const std::string UPDATE_FILE;
	static const std::string UPDATE_TMP_FILE;
	static const std::string PLUGINS_FOLDER;
	static const std::string RCCDC_PATH;
	static const std::string RCCDC_PACKAGE_FILE;
	static const std::string STEAM_USERDATA_PATH;
	static const std::string DECKY_SERVICE_PATH;

	static const std::string LOG_FILE_NAME;
	static const std::string LOG_RUNNER_FILE_NAME;
	static const std::string LOG_ORGB_FILE_NAME;

	static const std::string URL_GAME_CFG;
	static const std::string URL_PERF_PROF;
	static const std::string URL_DEC_BRIGHT;
	static const std::string URL_INC_BRIGHT;
	static const std::string URL_NEXT_EFF;

	static const std::string GAME_CFG;
	static const std::string PERF_PROF;
	static const std::string DEC_BRIGHT;
	static const std::string INC_BRIGHT;
	static const std::string NEXT_EFF;

	static const std::string SOCKET_FILE;

	static const std::vector<std::string> RCCDC_REQUIRED_PIP;
	static const std::string FLATPAK_MANGOHUD;
	static const std::string FLATPAK_MANGOHUD_OVERRIDE;

	static const std::string LOGGER_PATTERN;

	static const std::string RCDCC_SOCKET_PATH;

	static const std::string TRANSLATIONS_FILE;
	static const std::string CHANGELOG_URL;
};
