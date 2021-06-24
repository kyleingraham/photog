# photog_color.cpp generators
add_executable(color.generators photog_color.cpp photog_color.h)
target_link_libraries(color.generators
        PRIVATE
        Halide::Generator
        generator
        utils)

## From here we create Halide libraries for each export from photog_color.cpp
set(COLOR_GENERATORS
        photog_srgb_to_linear
        photog_rgb_to_linear
        photog_srgb_to_xyz
        photog_rgb_to_xyz
        photog_linear_to_srgb
        photog_linear_to_rgb
        photog_xyz_to_srgb
        photog_xyz_to_rgb
        photog_average
        photog_chromadapt)
set(COLOR_GENERATORS ${COLOR_GENERATORS} PARENT_SCOPE)

list(GET COLOR_GENERATORS 0 FIRST_GENERATOR)
set(COLOR_SHARED_RUNTIME ${FIRST_GENERATOR}.runtime)
set(COLOR_SHARED_RUNTIME ${COLOR_SHARED_RUNTIME} PARENT_SCOPE)

# TODO: Can we add optimization flags to these with target properties?
foreach (COLOR_GENERATOR IN LISTS COLOR_GENERATORS)
    if (${COLOR_GENERATOR} STREQUAL ${FIRST_GENERATOR})
        add_halide_library(${COLOR_GENERATOR} FROM color.generators
                GENERATOR ${COLOR_GENERATOR}
                AUTOSCHEDULER Halide::Adams2019
                PARAMS auto_schedule=true
                SCHEDULE ${COLOR_GENERATOR}_schedule
                HEADER ${COLOR_GENERATOR}_header)
    else()
        add_halide_library(${COLOR_GENERATOR} FROM color.generators
                GENERATOR ${COLOR_GENERATOR}
                USE_RUNTIME ${COLOR_SHARED_RUNTIME}
                AUTOSCHEDULER Halide::Adams2019
                PARAMS auto_schedule=true
                SCHEDULE ${COLOR_GENERATOR}_schedule
                HEADER ${COLOR_GENERATOR}_header)
    endif ()
    set_target_properties(${COLOR_GENERATOR}
            PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${${COLOR_GENERATOR}_header})
endforeach ()

# photog_color library (photog::photo_color target)
add_library(photog_color photog_color.cpp photog_color.h)
target_include_directories(photog_color
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog>")
target_link_libraries(photog_color
        PUBLIC Halide::Runtime # TODO: Do we need this public link?
        PRIVATE
        generator
        utils)
set_target_properties(photog_color
        PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/photog_color.h) # allows use of global install settings

# color library (photog::color target) - bundles photog_color library alongside color-related generator output
add_library(color INTERFACE)
target_link_libraries(color
        INTERFACE
        photog_color
        ${COLOR_GENERATORS})