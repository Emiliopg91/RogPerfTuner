cmake_minimum_required(VERSION 3.16)

project(RogControlCenter VERSION 4.0.4 LANGUAGES CXX)

add_subdirectory(submodules/httplib)

add_subdirectory(submodules/ixwebsocket)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(submodules/json)

add_subdirectory(submodules/spdlog)

add_subdirectory(submodules/OpenRGB-cppSDK)

add_subdirectory(RccCommons)

add_subdirectory(RccScripts)

add_subdirectory(RccCore)
