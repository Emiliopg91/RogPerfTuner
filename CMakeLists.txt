cmake_minimum_required(VERSION 3.16)

project(RogControlCenter VERSION 4.0.12 LANGUAGES CXX)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-funroll-loops -ftree-vectorize -fomit-frame-pointer -fmerge-all-constants )
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-pg)
endif()

add_compile_options(-O0 -Wno-unqualified-std-cast-call -Wextra -Werror -Wno-unused-result -Wno-deprecated-copy)

add_subdirectory(submodules/httplib)

add_subdirectory(submodules/ixwebsocket)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(submodules/json)

add_subdirectory(submodules/spdlog)

add_subdirectory(submodules/OpenRGB-cppSDK)

add_subdirectory(RogControlCenter)
