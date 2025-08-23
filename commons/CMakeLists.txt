cmake_minimum_required(VERSION 3.16)

# Si no está definido, forzamos Debug por defecto
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags específicos para cada tipo de build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -flto -Wno-unused-result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wno-unused-result)
endif()

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

add_library(RccCommons STATIC
    ${SOURCES}
)

target_include_directories(RccCommons PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBSECRET_INCLUDE_DIRS}
)

target_compile_options(RccCommons PRIVATE ${LIBSECRET_CFLAGS_OTHER})

target_link_libraries(RccCommons PUBLIC 
    spdlog::spdlog 
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
    ${LIBSECRET_LIBRARIES}
)
