add_subdirectory(generator)

add_library(photog_color color.cpp color.h)
target_include_directories(photog_color
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>") # TODO: Revisit when configuring installation.
target_link_libraries(photog_color
        PUBLIC Halide::Runtime
        PRIVATE photog_generator)

# color.cpp generators
add_executable(color.generators color.cpp color.h)
target_link_libraries(color.generators
        PRIVATE Halide::Generator photog_generator)

## From here we can add Halide libraries for each export
### photog_srgb_to_linear - Set implicit GeneratorParams. Required because Halide can't imply them from elsewhere.
add_halide_library(photog_srgb_to_linear FROM color.generators
        GENERATOR photog_srgb_to_linear
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true
        SCHEDULE photog_srgb_to_linear_schedule)

add_halide_library(photog_srgb_to_xyz FROM color.generators
        GENERATOR photog_srgb_to_xyz
        USE_RUNTIME photog_srgb_to_linear.runtime
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true
        SCHEDULE photog_srgb_to_xyz_schedule)

add_halide_library(photog_rgb_to_xyz FROM color.generators
        GENERATOR photog_rgb_to_xyz
        USE_RUNTIME photog_srgb_to_linear.runtime
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true
        SCHEDULE photog_rgb_to_xyz_schedule)

# Install
file(GLOB GENERATED_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/*.h")
install(FILES ${GENERATED_HEADERS} DESTINATION ${CMAKE_SOURCE_DIR}/dist/include)
install(TARGETS
        photog_srgb_to_linear
        photog_srgb_to_linear.runtime
        photog_srgb_to_xyz
        photog_rgb_to_xyz
        DESTINATION ${CMAKE_SOURCE_DIR}/dist/lib)