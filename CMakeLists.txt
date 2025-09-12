cmake_minimum_required(VERSION 3.16)

project(RogControlCenter VERSION 4.1.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "" FORCE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    #add_compile_options(-O3)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-pg -O0 )
endif()

add_compile_options(-Wno-unqualified-std-cast-call -Wextra -Werror -Wno-unused-result -Wno-deprecated-copy)

add_subdirectory(submodules/httplib)

add_subdirectory(submodules/ixwebsocket)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(submodules/json)

add_subdirectory(submodules/spdlog)

add_subdirectory(submodules/OpenRGB-cppSDK)

add_subdirectory(RogControlCenter)
