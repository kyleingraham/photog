# photog Generator library
add_library(photog_generator INTERFACE)
target_sources(photog_generator INTERFACE generator.h)
target_include_directories(photog_generator
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>") # TODO: Revisit when configuring installation.
if (APPLE)
    target_link_libraries(photog_generator
            INTERFACE Halide::Generator)
endif ()