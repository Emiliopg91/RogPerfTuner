cmake_minimum_required(VERSION 3.16)

project(RogPerfTuner VERSION 4.14.9 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "" FORCE)

find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    message(STATUS "CCache found: ${CCACHE_PROGRAM}")

    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
else()
    message(STATUS "ccache not found")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
        -O3
        -flto=full
        -march=native
        -ffunction-sections
        -fdata-sections
        -fvisibility=hidden
        -fno-unwind-tables
        -fno-asynchronous-unwind-tables
        -fno-semantic-interposition
        -fno-plt
        -fmerge-all-constants
    )

    add_link_options(
        -flto=thin
        -fuse-ld=lld
        -ffunction-sections
        -fdata-sections
        -Wl,--gc-sections
        -Wl,--icf=all
        -Wl,--threads=8
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -O0 
        -g3 
        -ggdb 
        -pipe 
        -fno-omit-frame-pointer
    )
    
    add_link_options(
        -fuse-ld=lld
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
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/httplib
)

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/magic_enum/include/magic_enum
)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/nlohmann-json/include
)

set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tools" FORCE)
add_subdirectory(submodules/yaml-cpp)

add_subdirectory(submodules/OpenRGB-cppSDK)

add_subdirectory(Framework)

add_subdirectory(RogPerfTuner)
