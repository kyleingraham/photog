include(GNUInstallDirs) # Make CMAKE_INSTALL_<dir> variables available

add_subdirectory(generator)

# photog_color.cpp generators
add_executable(color.generators photog_color.cpp photog_color.h)
target_link_libraries(color.generators
        PRIVATE Halide::Generator generator)

## From here we create Halide libraries for each export from photog_color.cpp
add_halide_library(photog_srgb_to_linear FROM color.generators
        GENERATOR photog_srgb_to_linear
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true
        SCHEDULE photog_srgb_to_linear_schedule
        HEADER photog_srgb_to_linear_header)
set_target_properties(photog_srgb_to_linear PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${photog_srgb_to_linear_header})

add_halide_library(photog_srgb_to_xyz FROM color.generators
        GENERATOR photog_srgb_to_xyz
        USE_RUNTIME photog_srgb_to_linear.runtime
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true
        SCHEDULE photog_srgb_to_xyz_schedule
        HEADER photog_srgb_to_xyz_header)
set_target_properties(photog_srgb_to_xyz PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${photog_srgb_to_xyz_header})

add_halide_library(photog_rgb_to_xyz FROM color.generators
        GENERATOR photog_rgb_to_xyz
        USE_RUNTIME photog_srgb_to_linear.runtime
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true
        SCHEDULE photog_rgb_to_xyz_schedule
        HEADER photog_rgb_to_xyz_header)
set_target_properties(photog_rgb_to_xyz PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${photog_rgb_to_xyz_header})

# photog_color library (photog::photo_color target)
add_library(photog_color photog_color.cpp photog_color.h)
target_include_directories(photog_color
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/photog>")
target_link_libraries(photog_color
        PUBLIC Halide::Runtime
        PRIVATE generator)
set_target_properties(photog_color PROPERTIES PUBLIC_HEADER photog_color.h) # allows use of global install settings

# color library (photog::color target) - bundles photog_color library alongside color-related generator output
add_library(color INTERFACE)
target_link_libraries(color
        INTERFACE
        photog_color
        photog_srgb_to_linear
        photog_srgb_to_linear.runtime
        photog_srgb_to_xyz
        photog_rgb_to_xyz)

# Project Install
## Target destinations for project
install(TARGETS
        color
        photog_color
        generator
        photog_srgb_to_linear
        photog_srgb_to_linear.runtime
        photog_srgb_to_xyz
        photog_rgb_to_xyz
        EXPORT photog_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/photog
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog
        PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/photog)

## Export details for project
install(EXPORT photog_targets
        FILE ${CMAKE_PROJECT_NAME}Targets.cmake
        NAMESPACE photog::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/photog/cmake/photog) # https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure