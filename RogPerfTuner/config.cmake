#########################
###### Application ######
#########################
add_definitions(-DM_APP_VERSION="4.14.10")
add_definitions(-DM_PLUGIN_VERSION="1.2.8")

#########################
########## CPU ##########
#########################
add_definitions(-DCPU_NAME="13th Gen Intel Core i9-13980HX")

#########################
########## GPU ##########
#########################
add_definitions(-DGPU_BRAND="nvidia")
add_definitions(-DGPU_NAME="NVIDIA Corporation AD104M [GeForce RTX™ 4080 Max-Q / Mobile]")
add_definitions(-DGPU_ENV="__GLX_VENDOR_LIBRARY_NAME=nvidia __NV_PRIME_RENDER_OFFLOAD=1 __VK_LAYER_NV_optimus=NVIDIA_only VK_LOADER_DRIVERS_SELECT=*nvidia*")

#########################
######## HEADERS ########
#########################
set(HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/dbus/asus/asus_base_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/dbus/asus/core/platform_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/armoury_base_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/proc_modules_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/sched_bore_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/ssd_scheduler_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/lib/udev_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/asusctl_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/flatpak.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/scxctl_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/breathing_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/dance_floor_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/digital_rain_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/drops_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/gaming.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/rainbow_wave.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/spectrum_cycle_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/starry_night_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/effects/static_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/tcp/open_rgb/open_rgb_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/unix_socket/rog_perf_tuner_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/unix_socket/steam_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/game_config_dialog.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/game_list.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/main_window.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/password_dialog.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/tray_icon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/yes_no_dialog.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/main/dev.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/main/flatpak.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/main/gui.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/main/run.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/main/socket.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/gpu_brand.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/gpu_info.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/rgb_brightness.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/usb_identifier.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/others/app_options.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/others/events.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/others/release_entry.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/others/semantic_version.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/performance/performance_profile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/performance/platform_profile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/performance/power_profile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/application.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/aura.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/game_entry.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/performance.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/platform.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/root_config.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/computer_type.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/mangohud_level.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/steam_game_config.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/steam_game_details.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/wine_sync_option.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/servers/socket_server.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/services/application_service.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/services/hardware_service.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/services/open_rgb_service.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/services/performance_service.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/services/steam_service.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/utils/configuration_wrapper.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/utils/constants.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/utils/event_bus_wrapper.hpp"
)

#########################
######## SOURCES ########
#########################
set(SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/dbus/asus/asus_base_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/dbus/asus/core/platform_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/armoury_base_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/proc_modules_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/sched_bore_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/ssd_scheduler_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/lib/udev_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/asusctl_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/flatpak.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/scxctl_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/abstract/abstract_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/breathing_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/dance_floor_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/digital_rain_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/drops_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/gaming.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/rainbow_wave.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/spectrum_cycle_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/starry_night_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/effects/static_effect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/tcp/open_rgb/open_rgb_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/unix_socket/rog_perf_tuner_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/unix_socket/steam_client.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/game_config_dialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/game_list.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/main_window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/password_dialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/tray_icon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/yes_no_dialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/servers/socket_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/services/application_service.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/services/hardware_service.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/services/open_rgb_service.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/services/performance_service.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/services/steam_service.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/configuration_wrapper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/constants.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/event_bus_wrapper.cpp"
)

##################################
########## ACPI_PROFILE ##########
##################################
set(ACPI_PROFILE ON)
add_definitions(-DACPI_PROFILE_FILE="/sys/firmware/acpi/platform_profile")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/power_profile_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/power_profile_client.cpp"
)

###############################
########## BAT_LIMIT ##########
###############################
set(BAT_LIMIT ON)
add_definitions(-DBAT_LIMIT_FILE="/sys/class/power_supply/BAT0/charge_control_end_threshold")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/battery_charge_limit_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/battery_charge_threshold.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/battery_charge_limit_client.cpp"
)

################################
########## BAT_STATUS ##########
################################
set(BAT_STATUS ON)
add_definitions(-DBAT_STATUS_FILE="/sys/class/power_supply/BAT0/status")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/battery_status_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/battery_status_client.cpp"
)

###################################
########## BOOST_CONTROL ##########
###################################
set(BOOST_CONTROL ON)
add_definitions(-DBOOST_CONTROL_FILE="/sys/devices/system/cpu/intel_pstate/no_turbo")
add_definitions(-DBOOST_CONTROL_OFF="1")
add_definitions(-DBOOST_CONTROL_ON="0")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/boost_control_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/boost_control_client.cpp"
)

################################
########## BOOT_SOUND ##########
################################
set(BOOT_SOUND ON)
add_definitions(-DBOOT_SOUND_FILE="/sys/class/firmware-attributes/asus-armoury/attributes/boot_sound")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/other/boot_sound_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/other/boot_sound_client.cpp"
)

#################################
########## FAN_CONTROL ##########
#################################
set(FAN_CONTROL ON)
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/fan_curve_editor.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/fan_curve_view.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/fan_curve_data.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/fan_curve.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/fan_curve_editor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/fan_curve_view.cpp"
)

###################################
########## INTEL_RAPL_UJ ##########
###################################
set(INTEL_RAPL_UJ ON)
add_definitions(-DINTEL_RAPL_UJ_FILE="/sys/class/powercap/intel-rapl:0/energy_uj")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/intel_rapl_uj_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/intel_rapl_uj_client.cpp"
)

################################
########## NTSYNC_MOD ##########
################################
set(NTSYNC_MOD ON)

##############################
########## NV_BOOST ##########
##############################
set(NV_BOOST ON)
add_definitions(-DNVIDIA_BOOST_FILE="/sys/class/firmware-attributes/asus-armoury/attributes/nv_dynamic_boost")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/nvidia/nv_boost_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/nvidia/nv_boost_client.cpp"
)

################################
########## NV_THERMAL ##########
################################
set(NV_THERMAL ON)
add_definitions(-DNVIDIA_THERMAL_FILE="/sys/class/firmware-attributes/asus-armoury/attributes/nv_temp_target")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/nvidia/nv_temp_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/nvidia/nv_temp_client.cpp"
)

##############################
########## PANEL_OD ##########
##############################
set(PANEL_OD ON)
add_definitions(-DPANEL_OD_FILE="/sys/class/firmware-attributes/asus-armoury/attributes/panel_overdrive")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/other/panel_overdrive_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/other/panel_overdrive_client.cpp"
)

#################################
########## PPT_PL1_SPL ##########
#################################
set(PPT_PL1_SPL ON)
add_definitions(-DPPT_PL1_SPL_FILE="/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl1_spl")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl1_spd_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl1_spd_client.cpp"
)

##################################
########## PPT_PL2_SPPT ##########
##################################
set(PPT_PL2_SPPT ON)
add_definitions(-DPPT_PL2_SPPT_FILE="/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl2_sppt")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.cpp"
)

######################################
########## SCALING_GOVERNOR ##########
######################################
set(SCALING_GOVERNOR ON)
add_definitions(-DSCALING_GOVERNOR_FILE="/sys/devices/system/cpu/cpu*/cpufreq/scaling_governor")
list(APPEND HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/scaling_governor_client.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/models/performance/cpu_governor.hpp"
)
list(APPEND SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/scaling_governor_client.cpp"
)

