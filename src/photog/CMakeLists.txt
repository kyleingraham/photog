# TODO: Exclude embedded doctest tests from release builds
set(color_headers include/photog/color.h)
set(support_headers generator.h utils.h)

# Executable we use for generating color's Halide libraries
add_executable(color_generators
        color_generators.cpp
        color_utils.cpp
        color_utils.h
        ${color_headers}
        ${support_headers})
target_include_directories(color_generators
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_link_libraries(color_generators
        PRIVATE
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

# TODO: Can we add optimization flags to these with target properties?
foreach (color_halide_library IN LISTS color_halide_libraries)
    if (${color_halide_library} STREQUAL ${first_halide_library})
        add_halide_library(${color_halide_library} FROM color_generators
                GENERATOR ${color_halide_library}
                AUTOSCHEDULER Halide::Adams2019
                PARAMS auto_schedule=true
                SCHEDULE ${color_halide_library}_schedule
                HEADER ${color_halide_library}_header)
    else ()
        add_halide_library(${color_halide_library} FROM color_generators
                GENERATOR ${color_halide_library}
                USE_RUNTIME ${shared_halide_runtime}
                AUTOSCHEDULER Halide::Adams2019
                PARAMS auto_schedule=true
                SCHEDULE ${color_halide_library}_schedule
                HEADER ${color_halide_library}_header)
    endif ()
    set_target_properties(${color_halide_library}
            PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${${color_halide_library}_header})
endforeach ()

# Internal access to all generated Halide libraries
add_library(color_halide_libraries_bundle INTERFACE)
target_link_libraries(color_halide_libraries_bundle
        INTERFACE
        ${color_halide_libraries}) # Linking any add_halide_library target brings in its headers and libraries

# Internal access to color utility functions
add_library(color_utils
        color_utils.cpp
        color_utils.h
        ${color_headers}
        ${support_headers})
target_include_directories(color_utils
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_link_libraries(color_utils
        PUBLIC
        Halide::Halide
        PRIVATE
        doctest::doctest)

# Public color library (photog::photog_color target)
add_library(color
        color.cpp
        ${color_headers}
        color_utils.cpp
        color_utils.h
        ${support_headers})
target_include_directories(color
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog>"
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
target_link_libraries(color
        PRIVATE
        ${color_halide_libraries}
        doctest::doctest
        Halide::Halide)
set_target_properties(color
        PROPERTIES
        PUBLIC_HEADER
        ${CMAKE_CURRENT_SOURCE_DIR}/include/photog/color.h
        OUTPUT_NAME
        "photog_color")
target_compile_definitions(color
        PRIVATE
        DOCTEST_CONFIG_DISABLE)