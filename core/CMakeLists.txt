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
    add_compile_options(-O3 -flto -Wno-unused-result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-pg -O0 -Wno-unused-result)
endif()

# -----------------------------
# Buscar paquetes de Qt6
# -----------------------------
find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets)


# -----------------------------
# Ejecutable principal
# -----------------------------
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(RogControlCenter ${SOURCES} ${HEADERS})

target_link_libraries(RogControlCenter PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    spdlog::spdlog 
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
    udev
    orgbsdk
    RccCommons 
)
