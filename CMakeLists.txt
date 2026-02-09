cmake_minimum_required(VERSION 3.16)

project(RogPerfTuner VERSION 4.11.5 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "" FORCE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
        -O3
        -march=native
        -funroll-loops
        -fstrict-aliasing
        -flto
    )

    add_link_options(
        -flto=full
        -s
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -O0
        -pg
    )
endif()

add_compile_options(
    -Wno-unqualified-std-cast-call 
    -Wextra 
    -Werror 
    -Wno-unused-result 
    -Wno-deprecated-copy 
    -Wno-deprecated-literal-operator
)

add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE
    "submodules/httplib/include/httplib.h"
)

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE
    "submodules/magic_enum/include/magic_enum"
)

add_library(nlohmann_json INTERFACE)
target_include_directories(magic_enum INTERFACE
    "submodules/nlohmann-json/include/nlohmann"
)

add_subdirectory(submodules/yaml-cpp)

add_subdirectory(submodules/OpenRGB-cppSDK)

add_subdirectory(Framework)

add_subdirectory(RogPerfTuner)