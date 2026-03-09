cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if(CMAKE_BUILD_TYPE STREQUAL "Release")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=full")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=full")
#endif()

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

if(DEV_MODE)
    message("Dev mode enabled")
    add_compile_definitions(DEV_MODE)
endif()

if(STEAM_SUPPORT)
    message("Steam support enabled")
    add_compile_definitions(STEAM_SUPPORT)
endif()

if(SCX_SUPPORT)
    message("Scx support enabled")
    add_compile_definitions(SCX_SUPPORT)
endif()

if(MANGOHUD_SUPPORT)
    message("MangoHud support enabled")
    add_compile_definitions(MANGOHUD_SUPPORT)
endif()

if(SWITCHEROO_SUPPORT)
    message("Switcheroo support enabled")
    add_compile_definitions(SWITCHEROO_SUPPORT)
endif()