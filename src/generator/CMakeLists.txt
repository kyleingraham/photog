# photog_generator library (photog::photog_generator target)
add_library(generator INTERFACE)
target_sources(generator
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/photog_generator.h>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog/photog_generator.h>")
target_include_directories(generator
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog>")
set_target_properties(generator PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/photog_generator.h)
if (APPLE)
    target_link_libraries(generator
            INTERFACE Halide::Generator)
endif ()