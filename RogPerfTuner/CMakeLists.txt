cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets Charts)
find_package(Qt6Keychain REQUIRED)

file(GLOB_RECURSE SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER SOURCES_RCC EXCLUDE REGEX ".*/src/main/helpers/.*")

file(GLOB_RECURSE HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

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
if(DEFINED ENV{IN_TEST})
    message("CONFIGURING FOR TESTS")
    set(IN_TEST_MODE ON)
endif()

find_program(STEAM_CMD steam)
set(STEAM_SUPPORT OFF)
if(STEAM_CMD OR IN_TEST_MODE)
    set(STEAM_SUPPORT ON)
endif()

find_program(SCXCTL_CMD scxctl)
set(SCX_SUPPORT OFF)
if(SCXCTL_CMD OR IN_TEST_MODE)
    set(SCX_SUPPORT ON)
endif()

find_program(MANGOHUD_CMD mangohud)
set(MANGOHUD_SUPPORT OFF)
if(MANGOHUD_CMD OR IN_TEST_MODE)
    set(MANGOHUD_SUPPORT ON)
endif()

find_program(SWITCHEROOCTL_CMD switcherooctl)
set(SWITCHEROO_SUPPORT OFF)
if(SWITCHEROOCTL_CMD OR IN_TEST_MODE)
    set(SWITCHEROO_SUPPORT ON)
endif()

set(DEV_MODE OFF)
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