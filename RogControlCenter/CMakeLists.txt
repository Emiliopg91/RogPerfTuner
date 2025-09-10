cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-flto)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ------------------------------
# Buscar paquetes de Qt6
# ------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets)

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
target_include_directories(RogControlCenter PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(RogControlCenter PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    spdlog::spdlog_header_only 
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


add_executable(NextProfile ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/performance/next_profile.cpp)
target_include_directories(NextProfile PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(NextProfile PRIVATE 
    httplib::httplib
)

add_executable(NextEffect ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/rgb/next_effect.cpp)
target_include_directories(NextEffect PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(NextEffect PRIVATE 
    httplib::httplib
)

add_executable(IncBrightness ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/rgb/increase_brightness.cpp)
target_include_directories(IncBrightness PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(IncBrightness PRIVATE 
    httplib::httplib
)

add_executable(DecBrightness ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/rgb/decrease_brightness.cpp)
target_include_directories(DecBrightness PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(DecBrightness PRIVATE 
    httplib::httplib
)

add_executable(SteamRunner ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/steam/run.cpp ${SOURCES_SHELL} ${SOURCES_LOGGER})
target_include_directories(SteamRunner PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(SteamRunner PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only 
)

add_executable(FlatpakWrapper ${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/steam/flatpak.cpp ${SOURCES_SHELL} ${SOURCES_LOGGER})
target_include_directories(FlatpakWrapper PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(FlatpakWrapper PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only 
)