cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=full")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=full")
endif()

# ------------------------------
# Buscar paquetes de Qt6
# ------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets Charts)

# ------------------------------ 
# Buscar paquetes de libsecret-1
# ------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

# ------------------------------
# Ejecutable principal
# ------------------------------
file(GLOB_RECURSE SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER SOURCES_RCC EXCLUDE REGEX ".*/src/scripts/.*")

file(GLOB_RECURSE HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(RogControlCenter ${SOURCES_RCC} ${HEADERS_RCC})
set_target_properties(RogControlCenter PROPERTIES AUTOMOC ON)
target_include_directories(RogControlCenter PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(RogControlCenter PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Charts
    spdlog::spdlog 
    nlohmann_json::nlohmann_json
    httplib::httplib
    ixwebsocket::ixwebsocket
    ${LIBSECRET_LIBRARIES}
    udev
    orgbsdk
)


# ------------------------------
# Scripts
# ------------------------------
file(GLOB_RECURSE SOURCES_SCT
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER SOURCES_SCT EXCLUDE REGEX ".*/src/scripts/.*")

file(GLOB_RECURSE HEADERS_SCT
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE SOURCES_SHELL
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shell/*.cpp"
)

file(GLOB_RECURSE SOURCES_LOGGER
    "${CMAKE_CURRENT_SOURCE_DIR}/src/logger/*.cpp"
)

file(GLOB_RECURSE SOURCES_FLATPAK
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/flatpak.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clients/shell/abstract/*.cpp"
)

file(GLOB_RECURSE SOURCES_UTILS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/constants.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/file_utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/string_utils.cpp"
)


add_executable(NextProfile ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/performance/next_profile.cpp ${SOURCES_UTILS})
target_include_directories(NextProfile PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(NextProfile PRIVATE 
    httplib::httplib
)

add_executable(NextEffect ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/rgb/next_effect.cpp ${SOURCES_UTILS})
target_include_directories(NextEffect PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(NextEffect PRIVATE 
    httplib::httplib
)

add_executable(IncBrightness ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/rgb/increase_brightness.cpp ${SOURCES_UTILS})
target_include_directories(IncBrightness PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(IncBrightness PRIVATE 
    httplib::httplib
)

add_executable(DecBrightness ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/rgb/decrease_brightness.cpp ${SOURCES_UTILS})
target_include_directories(DecBrightness PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(DecBrightness PRIVATE 
    httplib::httplib
)

add_executable(SteamRunner ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/steam/run.cpp ${SOURCES_SHELL} ${SOURCES_LOGGER} ${SOURCES_UTILS})
target_include_directories(SteamRunner PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(SteamRunner PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog 
)

add_executable(FlatpakWrapper ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/steam/flatpak.cpp ${SOURCES_SHELL} ${SOURCES_LOGGER} ${SOURCES_UTILS} ${SOURCES_FLATPAK})
target_include_directories(FlatpakWrapper PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(FlatpakWrapper PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog 
)