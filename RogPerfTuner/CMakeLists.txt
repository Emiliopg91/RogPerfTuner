cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=full")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=full")
endif()

#find_package(Doxygen REQUIRED)
#set(DOXYGEN_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
#set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
#configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
#
#add_custom_target(rcc-doc ALL
#    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#    COMMENT "Generating Doxygen documentation..."
#    VERBATIM
#)

# ------------------------------
# Buscar paquetes de Qt6
# ------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Widgets Charts)
find_package(Qt6Keychain REQUIRED)

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
list(FILTER SOURCES_RCC EXCLUDE REGEX ".*/src/main/helpers/.*")

file(GLOB_RECURSE HEADERS_RCC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(RogPerfTuner ${SOURCES_RCC} ${HEADERS_RCC})
set_target_properties(RogPerfTuner PROPERTIES AUTOMOC ON)
target_include_directories(RogPerfTuner PRIVATE ${LIBSECRET_INCLUDE_DIRS})
target_link_libraries(RogPerfTuner PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Charts
    spdlog::spdlog 
    yaml-cpp::yaml-cpp
    qt6keychain
    orgbsdk
    udev
)

if(IS_AURPKG)
    add_compile_definitions(IS_AURPKG)
endif()