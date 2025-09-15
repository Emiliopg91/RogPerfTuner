#pragma once

#include <sys/types.h>

#include <string>

class Constants {
  public:
	static const std::string APP_NAME;

	const static std::string APP_VERSION;
	const static std::string APPIMAGE_FILE;

	const static pid_t PID;

	const static std::string PLUGIN_VERSION;

	const static bool DEV_MODE;

	static const std::string HOME_DIR;

	static const std::string ASSETS_DIR;

	static const std::string USR_SHARE_OCL_DIR;
	static const std::string UDEV_RULES;
	static const std::string USR_SHARE_VK_DIR;
	static const std::string TMP_UDEV_PATH;
	static const std::string LOCK_FILE;

	static const std::string ASSETS_BIN_DIR;
	static const std::string ASSET_ICONS_DIR;
	static const std::string ASSET_ICON_FILE;
	static const std::string ORGB_DIR;
	static const std::string ORGB_UDEV_PATH;
	static const std::string ORGB_PATH;
	static const std::string RCCDC_ASSET_PATH;

	static const std::string AUTOSTART_FILE;
	static const std::string APP_DRAW_FILE;
	static const std::string BIN_DIR;
	static const std::string BIN_APPLICATION_DIR;
	static const std::string LAUNCHER_FILE;
	static const std::string WRAPPER_PATH;
	static const std::string CONFIG_DIR;
	static const std::string CONFIG_FILE;
	static const std::string ICONS_DIR;
	static const std::string ICON_FILE;
	static const std::string ICON_45_FILE;
	static const std::string LIB_DIR;
	static const std::string LIB_VK_DIR;
	static const std::string LIB_OCL_DIR;
	static const std::string LOG_DIR;
	static const std::string LOG_OLD_DIR;
	static const std::string LOG_ORGB_FILE;
	static const std::string USER_PLUGIN_DIR;
	static const std::string UPDATE_DIR;
	static const std::string UPDATE_FILE;
	static const std::string UPDATE_TMP_FILE;
	static const std::string PLUGINS_FOLDER;
	static const std::string RCCDC_PATH;
	static const std::string RCCDC_PACKAGE_FILE;
	static const std::string DECKY_SERVICE_PATH;

	static const std::string LOG_FILE_NAME;
	static const std::string LOG_RUNNER_FILE_NAME;

	static const std::string URL_GAME_CFG;
	static const std::string URL_PERF_PROF;
	static const std::string URL_RENICE;
	static const std::string URL_DEC_BRIGHT;
	static const std::string URL_INC_BRIGHT;
	static const std::string URL_NEXT_EFF;

	static const int HTTP_PORT;
	static const int WS_PORT;
};
