add_library(RccCommons INTERFACE)

target_include_directories(RccCommons INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBSECRET_INCLUDE_DIRS}
)