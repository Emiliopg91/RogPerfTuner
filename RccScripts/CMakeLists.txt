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
    add_compile_options(-O3 -flto -Wextra -Werror -Wno-unused-result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-pg -O0 -Wextra -Werror -Wno-unused-result)
endif()

add_executable(NextEffect "${CMAKE_CURRENT_SOURCE_DIR}/src/rgb/next_effect.cpp")
target_link_libraries(NextEffect PRIVATE
    httplib::httplib
    spdlog::spdlog 
    spdlog::spdlog_header_only
    RccCommons
)

add_executable(IncBrightness "${CMAKE_CURRENT_SOURCE_DIR}/src/rgb/increase_brightness.cpp")
target_link_libraries(IncBrightness PRIVATE 
    httplib::httplib
    spdlog::spdlog 
    spdlog::spdlog_header_only
    RccCommons
)

add_executable(DecBrightness "${CMAKE_CURRENT_SOURCE_DIR}/src/rgb/decrease_brightness.cpp")
target_link_libraries(DecBrightness PRIVATE 
    httplib::httplib
    spdlog::spdlog 
    spdlog::spdlog_header_only
    RccCommons
)

add_executable(NextProfile "${CMAKE_CURRENT_SOURCE_DIR}/src/performance/next_profile.cpp")
target_link_libraries(NextProfile PRIVATE 
    httplib::httplib
    spdlog::spdlog 
    spdlog::spdlog_header_only
    RccCommons
)

add_executable(SteamRunner "${CMAKE_CURRENT_SOURCE_DIR}/src/steam/run.cpp")
target_link_libraries(SteamRunner PRIVATE 
    nlohmann_json::nlohmann_json
    httplib::httplib
    spdlog::spdlog 
    spdlog::spdlog_header_only
    RccCommons
)
