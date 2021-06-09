# photog_utils library
add_library(utils INTERFACE)
target_sources(utils
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/photog_utils.h>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog/photog_utils.h>")
target_include_directories(utils
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog>")
set_target_properties(utils PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/photog_utils.h)
target_link_libraries(utils
        INTERFACE Halide::Halide)