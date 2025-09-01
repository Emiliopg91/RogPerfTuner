
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(RccCommons STATIC ${SOURCES})

target_include_directories(RccCommons PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBSECRET_INCLUDE_DIRS}
)

target_link_libraries(RccCommons PUBLIC nlohmann_json::nlohmann_json)