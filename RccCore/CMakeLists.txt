cmake_minimum_required(VERSION 3.16)

# Si no está definido, forzamos Debug por defecto
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Flags específicos para cada tipo de build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -flto -Wextra -Werror -Wno-unused-result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-pg -O0 -Wextra -Werror -Wno-unused-result)
endif()

# -----------------------------
# Buscar paquetes de Qt6
# -----------------------------
find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets)


# -----------------------------
# Ejecutable principal
# -----------------------------
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(GENERATED_DEVICES ${CMAKE_CURRENT_BINARY_DIR}/include/clients/tcp/open_rgb/compatible_devices.hpp)
add_custom_command(
    OUTPUT ${GENERATED_DEVICES}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/clients/tcp/open_rgb
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../resources/preload/compatible_devices.py
            ${CMAKE_CURRENT_SOURCE_DIR}/../assets/OpenRGB/usr/lib/udev/rules.d/60-openrgb.rules
            ${GENERATED_DEVICES}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../assets/OpenRGB/usr/lib/udev/rules.d/60-openrgb.rules
    COMMENT "Generating compatible_devices.hpp from 60-openrgb.rules"
)
add_custom_target(generate_devices DEPENDS ${GENERATED_DEVICES})

set(GENERATED_TRANSLATIONS ${CMAKE_CURRENT_BINARY_DIR}/src/translator/translations.cpp)
add_custom_command(
    OUTPUT ${GENERATED_TRANSLATIONS}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/src/translator
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../resources/preload/translations.py
            ${CMAKE_CURRENT_SOURCE_DIR}/../resources/translations.json
            ${GENERATED_TRANSLATIONS}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../resources/translations.json
    COMMENT "Generating translations.cpp from translations.json asset"
)
add_custom_target(generate_translations DEPENDS ${GENERATED_TRANSLATIONS})

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER SOURCES EXCLUDE REGEX ".*/src/scripts/.*")

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(RogControlCenter ${SOURCES} ${HEADERS})
add_dependencies(RogControlCenter generate_devices)
add_dependencies(RogControlCenter generate_translations)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

target_include_directories(RogControlCenter PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(RogControlCenter PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    spdlog::spdlog 
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
    httplib::httplib
    ixwebsocket::ixwebsocket
    ${LIBSECRET_LIBRARIES}
    udev
    orgbsdk
    RccCommons
)
