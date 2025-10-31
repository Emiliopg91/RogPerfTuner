cmake_minimum_required(VERSION 3.16)

project(RogPerfTuner VERSION 4.6.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "" FORCE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-pg -O0 )
endif()

add_compile_options(
    -Wno-unqualified-std-cast-call 
    -Wextra 
    -Werror 
    -Wno-unused-result 
    -Wno-deprecated-copy 
    -Wno-deprecated-literal-operator
)

add_subdirectory(submodules/OpenRGB-cppSDK)

add_subdirectory(RogPerfTuner)
