cmake_minimum_required(VERSION 3.16)

project(Framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS DBus Widgets)
file(GLOB_RECURSE SOURCES_LIB
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE HEADERS_LIB
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_library(Framework STATIC
    ${SOURCES_LIB}
    ${HEADERS_LIB}
)

set_target_properties(Framework PROPERTIES AUTOMOC ON)

target_compile_definitions(Framework PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_link_libraries(Framework PUBLIC 
    magic_enum
    nlohmann_json
    OpenSSL::SSL 
    OpenSSL::Crypto
    Qt6::DBus
    Qt6::Widgets
    yaml-cpp::yaml-cpp
)

target_include_directories(Framework
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
