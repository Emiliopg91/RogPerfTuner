cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets Charts)
find_package(Qt6Keychain REQUIRED)

file(GLOB_RECURSE SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

string(ASCII 27 Esc)
if(DEFINED ENV{CMAKE_COLOR} AND "$ENV{CMAKE_COLOR}" STREQUAL "1")
    set(GREEN "${Esc}[32m")
    set(RED   "${Esc}[31m")
    set(RESET "${Esc}[0m")
    set(BOLD  "${Esc}[1m")
else()
    set(GREEN "")
    set(RED "")
    set(RESET "")
    set(BOLD  "")
endif()

function(handle_feature NAME VAR)
    if(${VAR})
        message("│ ${NAME} │   ${GREEN}YES${RESET}   │")
        add_compile_definitions(${VAR})
    else()
        message("│ ${NAME} │   ${RED}NO${RESET}    │")
    endif()
endfunction()

set(DEV_MODE OFF)
set(BOOST_CONTROL OFF)
set(INTEL_RAPL_UJ OFF)
set(PPT_PL1_SPL OFF)
set(PPT_PL2_SPPT OFF)
set(PPT_PL3_FPPT OFF)
set(NV_BOOST OFF)
set(NV_THERMAL OFF)
set(BOOT_SOUND OFF)
set(BAT_LIMIT OFF)
set(PANEL_OD OFF)
set(FAN_CONTROL OFF)

message("Checking features...")

if(DEFINED ENV{DEV_MODE})
    set(DEV_MODE ON)
endif()

if(EXISTS "/sys/devices/system/cpu/intel_pstate/no_turbo")
    set(BOOST_CONTROL ON)
    add_definitions(-DBOOST_CONTROL_FILE="/sys/devices/system/cpu/intel_pstate/no_turbo")
    add_definitions(-DBOOST_CONTROL_ON="0")
    add_definitions(-DBOOST_CONTROL_OFF="1")
elseif(EXISTS "/sys/devices/system/cpu/cpufreq/boost")
    set(BOOST_CONTROL ON)
    add_definitions(-DBOOST_CONTROL_FILE="/sys/devices/system/cpu/cpufreq/boost")
    add_definitions(-DBOOST_CONTROL_ON="1")
    add_definitions(-DBOOST_CONTROL_OFF="0")
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/boost_control_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/boost_control_client.cpp"
    )
endif()

if(EXISTS "/sys/class/powercap/intel-rapl:0/energy_uj")
    set(INTEL_RAPL_UJ ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/intel_rapl_uj_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/intel_rapl_uj_client.cpp"
    )
endif()

if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl1_spl/current_value")
    set(PPT_PL1_SPL ON)

    if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl2_sppt/current_value")
        set(PPT_PL2_SPPT ON)

        if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl3_fppt/current_value")
            set(PPT_PL3_FPPT ON)
        else()
            list(REMOVE_ITEM HEADERS_RCC
                "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.hpp"
            )
            list(REMOVE_ITEM SOURCES_RCC
                "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.cpp"
            )
        endif()
    else()
        list(REMOVE_ITEM HEADERS_RCC
            "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.hpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.hpp"
        )
        list(REMOVE_ITEM SOURCES_RCC
            "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.cpp"
        )
    endif()
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl1_spd_client.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl1_spd_client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.cpp"
    )
endif()

if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/nv_dynamic_boost/current_value")
    set(NV_BOOST ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/nvidia/nv_boost_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/nvidia/nv_boost_client.cpp"
    )
endif()

if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/nv_dynamic_boost/current_value")
    set(NV_THERMAL ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/nvidia/nv_temp_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/nvidia/nv_temp_client.cpp"
    )
endif()

if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/boot_sound/current_value")
    set(BOOT_SOUND ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/other/boot_sound_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/other/boot_sound_client.cpp"
    )
endif()

if(EXISTS "/sys/class/firmware-attributes/asus-armoury/attributes/panel_overdrive/current_value")
    set(PANEL_OD ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/firmware/asus-armoury/other/panel_overdrive_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/firmware/asus-armoury/other/panel_overdrive_client.cpp"
    )
endif()

if(EXISTS "/sys/class/power_supply/BAT0/charge_control_end_threshold")
    set(BAT_LIMIT ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/battery_charge_limit_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/battery_charge_limit_client.cpp"
    )
endif()

execute_process(
    COMMAND bash -c "asusctl fan-curve --get-enabled | wc -l"
    OUTPUT_VARIABLE FAN_CURVE_COUNT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(FAN_CURVE_COUNT GREATER 0)
    set(FAN_CONTROL ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/fan_curve_editor.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/fan_curve_view.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/hardware/fan_curve_data.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/fan_curve.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/fan_curve_editor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/fan_curve_view.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/models/hardware/fan_curve_data.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/models/settings/fan_curve.cpp"
    )
endif()


message("┌──────────────────┬─────────┐")
message("│     ${BOLD}Feature${RESET}      │ ${BOLD}Enabled${RESET} │")
message("├──────────────────┼─────────┤")
handle_feature("Dev mode        " DEV_MODE)
handle_feature("Boost control   " BOOST_CONTROL)
handle_feature("Intel Rapl UJ   " INTEL_RAPL_UJ)
handle_feature("TDP PL1 SPD     " PPT_PL1_SPL)
handle_feature("TDP PL2 SPPT    " PPT_PL2_SPPT)
handle_feature("TDP PL3 FPPT    " PPT_PL3_FPPT)
handle_feature("Nvidia Boost    " NV_BOOST)
handle_feature("Nvidia Thermal  " NV_THERMAL)
handle_feature("Boot sound      " BOOT_SOUND)
handle_feature("Panel Overdrive " PANEL_OD)
handle_feature("Battery limit   " BAT_LIMIT)
handle_feature("Fan control     " FAN_CONTROL)
message("└──────────────────┴─────────┘")


add_executable(RogPerfTuner ${SOURCES_RCC} ${HEADERS_RCC})
set_target_properties(RogPerfTuner PROPERTIES AUTOMOC ON)
target_include_directories(RogPerfTuner PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(RogPerfTuner PRIVATE 
    Framework
    magic_enum
    orgbsdk
    qt6keychain
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Charts
    udev 
    yaml-cpp::yaml-cpp
)