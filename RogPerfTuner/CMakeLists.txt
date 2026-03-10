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

set(IN_TEST_MODE OFF)
set(DEV_MODE OFF)
set(MANGOHUD_SUPPORT OFF)
set(SCX_SUPPORT OFF)
set(STEAM_SUPPORT OFF)
set(SWITCHEROO_SUPPORT OFF)

if(DEFINED ENV{IN_TEST})
    message("CONFIGURING FOR TESTS")
    set(IN_TEST_MODE ON)
endif()

find_program(STEAM_CMD steam)
if(STEAM_CMD OR IN_TEST_MODE)
    set(STEAM_SUPPORT ON)

    find_program(MANGOHUD_CMD mangohud)
    find_program(SWITCHEROOCTL_CMD switcherooctl)

else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/flatpak.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/intel_rapl_uj_client.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/switcherooctl_client.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/unix_socket/steam_client.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/game_config_dialog.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/gui/game_list.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/main/flatpak.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/main/run.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/settings/game_entry.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/computer_type.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/mangohud_level.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/steam_game_config.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/steam_game_details.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/wine_sync_option.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/services/steam_service.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/flatpak.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/intel_rapl_uj_client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/switcherooctl_client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/unix_socket/steam_client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/game_config_dialog.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/game_list.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/services/steam_service.cpp"
    )
endif()

if(SWITCHEROOCTL_CMD OR IN_TEST_MODE)
    set(SWITCHEROO_SUPPORT ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/switcherooctl_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/switcherooctl_client.cpp"
    )
endif()

if(MANGOHUD_CMD OR IN_TEST_MODE)
    set(MANGOHUD_SUPPORT ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/models/steam/mangohud_level.hpp"
    )
endif()

find_program(SCXCTL_CMD scxctl)
if(SCXCTL_CMD OR IN_TEST_MODE)
    set(SCX_SUPPORT ON)
else()
    list(REMOVE_ITEM HEADERS_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/file/sched_bore_client.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/clients/shell/scxctl_client.hpp"
    )
    list(REMOVE_ITEM SOURCES_RCC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/file/sched_bore_client.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/scxctl_client.cpp"
    )
endif()

if(DEFINED ENV{DEV_MODE} OR IN_TEST_MODE)
    set(DEV_MODE ON)
endif()

message("┌────────────┬─────────┐")
message("│  ${BOLD}Feature${RESET}   │ ${BOLD}Enabled${RESET} │")
message("├────────────┼─────────┤")
handle_feature("Dev mode  " DEV_MODE)
handle_feature("MangoHud  " MANGOHUD_SUPPORT)
handle_feature("SCX       " SCX_SUPPORT)
handle_feature("Steam     " STEAM_SUPPORT)
handle_feature("Switcheroo" SWITCHEROO_SUPPORT)
message("└────────────┴─────────┘")


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