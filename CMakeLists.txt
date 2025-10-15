cmake_minimum_required(VERSION 3.16)

project(RogPerfTuner VERSION 4.4.9 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "" FORCE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")

    add_compile_options(
        -funroll-loops
        -fstrict-aliasing
        -ftree-vectorize
        -fno-math-errno
    )

    if(DEFINED ENV{USE_NATIVE_OPTIMIZATION})
        message(STATUS "Native optimization enabled via environment variable.")
        add_compile_options(-march=native)
    endif()
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
