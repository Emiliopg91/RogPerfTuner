cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets Charts)
find_package(Qt6Keychain REQUIRED)

execute_process(
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/../resources/scripts/cmake_cfg.py
    ERROR_VARIABLE error
    RESULT_VARIABLE result
)
include("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")

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

message("┌───────────────────┬─────────┐")
message("│      ${BOLD}Feature${RESET}      │ ${BOLD}Enabled${RESET} │")
message("├───────────────────┼─────────┤")
handle_feature("Dev mode         " DEV_MODE)
handle_feature("ACPI profiles    " ACPI_PROFILE)
handle_feature("Battery limit    " BAT_LIMIT)
handle_feature("Battery status   " BAT_STATUS)
handle_feature("Boost control    " BOOST_CONTROL)
handle_feature("Boot sound       " BOOT_SOUND)
handle_feature("Fan control      " FAN_CONTROL)
handle_feature("Intel RAPL UJ    " INTEL_RAPL_UJ)
handle_feature("Nvidia boost     " NV_BOOST)
handle_feature("Nvidia thermal   " NV_THERMAL)
handle_feature("Panel overdrive  " PANEL_OD)
handle_feature("Scaling governor " SCALING_GOVERNOR)
handle_feature("TDP PL1 SPD      " PPT_PL1_SPL)
handle_feature("TDP PL2 SPPT     " PPT_PL2_SPPT)
handle_feature("TDP PL3 FPPT     " PPT_PL3_FPPT)
message("└───────────────────┴─────────┘")

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