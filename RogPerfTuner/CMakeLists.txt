cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if(CMAKE_BUILD_TYPE STREQUAL "Release")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=full")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=full")
#endif()

find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets Charts)
find_package(Qt6Keychain REQUIRED)

add_library(magic_enum INTERFACE)
target_include_directories(magic_enum INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../submodules/magic_enum/include/magic_enum"
)

file(GLOB_RECURSE SOURCES_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER SOURCES_RCC EXCLUDE REGEX ".*/src/main/helpers/.*")

file(GLOB_RECURSE HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(RogPerfTuner ${SOURCES_RCC} ${HEADERS_RCC})
set_target_properties(RogPerfTuner PROPERTIES AUTOMOC ON)
target_include_directories(RogPerfTuner PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(RogPerfTuner PRIVATE 
    magic_enum
    orgbsdk
    qt6keychain
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Charts
    udev
    yaml-cpp::yaml-cpp
)

if(DEV_MODE)
    add_compile_definitions(DEV_MODE)
endif()

if(DEFINED AUR_HELPER)
    message(STATUS "Detected AUR helper: ${AUR_HELPER}")
    add_compile_definitions(AUR_HELPER="${AUR_HELPER}")
else()
    message(STATUS "No AUR_HELPER defined.")
endif()