#include "utils/constants.hpp"

#include <limits.h>
#include <unistd.h>

#ifdef DEV_MODE
#include "file_utils.hpp"
#endif

const std::string Constants::APP_NAME  = "RogPerfTuner";
const std::string Constants::EXEC_NAME = "rog-perf-tuner";

const std::string Constants::APP_VERSION = "4.10.9-1";

const std::string Constants::PLUGIN_VERSION = "1.2.7";

#ifdef DEV_MODE
const std::string Constants::APP_DRAW_FILE = FileUtils::getCWD() + "/resources/" + Constants::APP_NAME + ".desktop";
const std::string Constants::ASSETS_DIR	   = FileUtils::getCWD() + "/build/assets";
const std::string Constants::UDEV_RULES	   = ASSETS_DIR + "/OpenRGB/60-openrgb.rules";
#else
const std::string Constants::APP_DRAW_FILE = "/usr/share/applications/" + EXEC_NAME + ".desktop";
const std::string Constants::ASSETS_DIR	   = "/usr/share/" + EXEC_NAME;
const std::string Constants::UDEV_RULES	   = "/usr/lib/udev/rules.d/60-" + EXEC_NAME + ".rules";
#endif

const std::string Constants::LOG_FILE_NAME		  = APP_NAME;
const std::string Constants::LOG_RUNNER_FILE_NAME = "Runner";
const std::string Constants::LOG_ORGB_FILE_NAME	  = "OpenRGB";

const std::string Constants::USR_SHARE_OCL_DIR = "/etc/OpenCL/vendors/";
const std::string Constants::USR_SHARE_VK_DIR  = "/usr/share/vulkan/icd.d/";
const std::string Constants::LOCK_FILE		   = "/tmp/" + APP_NAME + ".lock";
const std::string Constants::SOCKET_FILE	   = "/tmp/" + APP_NAME + ".sock";

const std::string Constants::ASSETS_BIN_DIR		= ASSETS_DIR + "/bin";
const std::string Constants::ASSET_ICONS_DIR	= ASSETS_DIR + "/icons";
const std::string Constants::ASSET_ICON_FILE	= ASSETS_DIR + "/icons/icon.svg";
const std::string Constants::ASSET_ICON_45_FILE = ASSETS_DIR + "/icons/icon-45x45.png";
const std::string Constants::ORGB_DIR			= ASSETS_DIR + "/OpenRGB";
const std::string Constants::ORGB_PATH			= ASSETS_DIR + "/OpenRGB/openrgb";
const std::string Constants::RCCDC_ASSET_PATH	= ASSETS_DIR + "/RccDeckyCompanion";
const std::string Constants::TRANSLATIONS_FILE	= ASSETS_DIR + "/translations.yaml";

const std::string Constants::BIN_DIR				  = HOME_DIR + "/." + APP_NAME + "/bin";
const std::string Constants::BIN_APPLICATION_DIR	  = HOME_DIR + "/." + APP_NAME + "/bin/application";
const std::string Constants::BIN_PERFORMANCE_DIR	  = HOME_DIR + "/." + APP_NAME + "/bin/performance";
const std::string Constants::NEXT_PROFILE_PATH		  = HOME_DIR + "/." + APP_NAME + "/bin/performance/nextProfile";
const std::string Constants::BIN_RGB_DIR			  = HOME_DIR + "/." + APP_NAME + "/bin/rgb";
const std::string Constants::DECREASE_BRIGHTNESS_PATH = HOME_DIR + "/." + APP_NAME + "/bin/rgb/decreaseBrightness";
const std::string Constants::INCREASE_BRIGHTNESS_PATH = HOME_DIR + "/." + APP_NAME + "/bin/rgb/increaseBrightness";
const std::string Constants::NEXT_EFFECT_PATH		  = HOME_DIR + "/." + APP_NAME + "/bin/rgb/nextEffect";
const std::string Constants::BIN_STEAM_DIR			  = HOME_DIR + "/." + APP_NAME + "/bin/steam";
const std::string Constants::STEAM_WRAPPER_PATH		  = HOME_DIR + "/." + APP_NAME + "/bin/steam/run";
const std::string Constants::FLATPAK_WRAPPER_PATH	  = HOME_DIR + "/." + APP_NAME + "/bin/steam/flatpak";
const std::string Constants::CONFIG_DIR				  = HOME_DIR + "/." + APP_NAME + "/config";
const std::string Constants::CONFIG_FILE			  = "config.yaml";
const std::string Constants::LOGOS_DIR				  = HOME_DIR + "/." + APP_NAME + "/logos";
const std::string Constants::LIB_DIR				  = HOME_DIR + "/." + APP_NAME + "/lib";
const std::string Constants::LIB_VK_DIR				  = HOME_DIR + "/." + APP_NAME + "/lib/vk/icd.d/";
const std::string Constants::LIB_OCL_DIR			  = HOME_DIR + "/." + APP_NAME + "/lib/ocl/icd.d/";
const std::string Constants::LOG_DIR				  = HOME_DIR + "/." + APP_NAME + "/logs";
const std::string Constants::LOG_OLD_DIR			  = HOME_DIR + "/." + APP_NAME + "/logs/old";
const std::string Constants::USER_PLUGIN_DIR		  = HOME_DIR + "/." + APP_NAME + "/plugin";
const std::string Constants::AUTOSTART_FILE			  = HOME_DIR + "/.config/autostart/" + APP_NAME + ".desktop";
const std::string Constants::STEAM_USERDATA_PATH	  = HOME_DIR + "/.steam/steam/userdata";
const std::string Constants::RCDCC_SOCKET_PATH		  = HOME_DIR + "/homebrew/data/RCCDeckyCompanion/socket";
const std::string Constants::PLUGINS_FOLDER			  = HOME_DIR + "/homebrew/plugins";
const std::string Constants::RCCDC_PATH				  = HOME_DIR + "/homebrew/plugins/RCCDeckyCompanion";
const std::string Constants::RCCDC_PACKAGE_FILE		  = HOME_DIR + "/homebrew/plugins/RCCDeckyCompanion/package.json";
const std::string Constants::DECKY_SERVICE_PATH		  = HOME_DIR + "/homebrew/services/PluginLoader";

const std::string Constants::GAME_CFG	= "gameConfig";
const std::string Constants::PERF_PROF	= "nexPerformanceProfile";
const std::string Constants::DEC_BRIGHT = "decRgbBrightness";
const std::string Constants::INC_BRIGHT = "incRgbBrightness";
const std::string Constants::NEXT_EFF	= "nextRgbEffect";

const std::vector<std::string> Constants::RCCDC_REQUIRED_PIP = {"asyncio", "pyyaml"};

const std::string Constants::FLATPAK_MANGOHUD		   = "org.freedesktop.Platform.VulkanLayer.MangoHud/x86_64/24.08";
const std::string Constants::FLATPAK_MANGOHUD_OVERRIDE = "--filesystem=xdg-config/MangoHud:ro";

const std::string Constants::CHANGELOG_URL = "https://github.com/Emiliopg91/RogPerfTuner/raw/refs/heads/main/changelog.yaml";
