set(color_headers include/photog/color.h)
set(support_source constants.h generator.h utils.cpp utils.h)

add_library(definitions INTERFACE)
target_compile_definitions(definitions
        INTERFACE
        LAYOUT="${photog_IMAGE_LAYOUT}"
        X_EXTENT_ESTIMATE=${photog_IMAGE_WIDTH_ESTIMATE}
        Y_EXTENT_ESTIMATE=${photog_IMAGE_HEIGHT_ESTIMATE})

# Executable we use for generating color's Halide libraries
add_executable(color_generators
        color_generators.cpp
        color_utils.cpp
        color_utils.h
        ${color_headers}
        ${support_source})
target_include_directories(color_generators
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_link_libraries(color_generators
        PRIVATE
        definitions
        doctest::doctest
        Halide::Generator)
target_compile_definitions(color_generators
        PUBLIC
        DOCTEST_CONFIG_DISABLE) # Prevents configuring doctest. We have a test runner elsewhere.

## From here we create Halide libraries for each registered generator
set(color_halide_libraries
        photog_srgb_to_linear
        photog_rgb_to_linear
        photog_srgb_to_xyz
        photog_rgb_to_xyz
        photog_linear_to_srgb
        photog_linear_to_rgb
        photog_xyz_to_srgb
        photog_xyz_to_rgb
        photog_average
        photog_chromadapt_impl)
set(COLOR_HALIDE_LIBRARIES ${color_halide_libraries} PARENT_SCOPE)

list(GET color_halide_libraries 0 first_halide_library)
set(shared_halide_runtime ${first_halide_library}.runtime)
set(SHARED_HALIDE_RUNTIME ${shared_halide_runtime} PARENT_SCOPE)

# TODO: Do optimization flags to affect these targets?
foreach (color_halide_library IN LISTS color_halide_libraries)
    if (${color_halide_library} STREQUAL ${first_halide_library})
        add_halide_library(${color_halide_library} FROM color_generators
                GENERATOR ${color_halide_library}
                AUTOSCHEDULER Halide::Adams2019
                PARAMS layout=${photog_IMAGE_LAYOUT}
                SCHEDULE ${color_halide_library}_schedule
                HEADER ${color_halide_library}_header)
    else ()
        add_halide_library(${color_halide_library} FROM color_generators
                GENERATOR ${color_halide_library}
                USE_RUNTIME ${shared_halide_runtime}
                AUTOSCHEDULER Halide::Adams2019
                PARAMS layout=${photog_IMAGE_LAYOUT}
                SCHEDULE ${color_halide_library}_schedule
                HEADER ${color_halide_library}_header)
    endif ()
    # We don't need to set install include directories here because we don't use these headers after install
    # Revisit if these headers are to be exposed to the end-user
endforeach ()

# Internal access to all generated color Halide libraries
add_library(color_halide_libraries_bundle INTERFACE)
target_link_libraries(color_halide_libraries_bundle
        INTERFACE
        ${color_halide_libraries}) # Linking any add_halide_library target brings in its headers and libraries

# Internal access to color utility functions
add_library(color_utils
        ${color_headers}
        color_utils.cpp
        color_utils.h
        ${support_source})
target_include_directories(color_utils
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_link_libraries(color_utils
        PRIVATE
        definitions
        doctest::doctest
        PUBLIC
        Halide::Halide)

# Public color library (photog::color target)
add_library(color
        color.cpp
        ${color_headers}
        color_utils.cpp
        color_utils.h
        ${support_source})
target_include_directories(color
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog/public>" # Include starts from here
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_link_libraries(color
        PRIVATE
        ${color_halide_libraries}
        definitions
        doctest::doctest
        Halide::Halide)
set_target_properties(color
        PROPERTIES
        PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/photog/color.h
        OUTPUT_NAME "photog_color"
        VERSION ${${CMAKE_PROJECT_NAME}_VERSION}
        SOVERSION ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}
        INTERFACE_color_MAJOR_VERSION ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}
        COMPATIBLE_INTERFACE_STRING color_MAJOR_VERSION)
target_compile_definitions(color
        PRIVATE
        DOCTEST_CONFIG_DISABLE)