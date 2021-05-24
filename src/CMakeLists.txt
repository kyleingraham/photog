# Halide bundles all of their headers into one -> Halide.h
# We won't do the same here. We'll start with the custom generator and later add generated headers.
# photog custom generator library
add_library(photog_generator photog_generator.cpp photog_generator.h)
target_include_directories(photog_generator
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>") # revisit when configuring installation
if (APPLE)
    target_link_libraries(photog_generator
            PUBLIC Halide::Generator) # https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#transitive-usage-requirements
endif ()

# General photog library

# color.cpp generators
add_executable(color.generators color.cpp)
target_link_libraries(color.generators
        PRIVATE Halide::Generator photog_generator)

## From here we can add Halide libraries for each export
### srgb_to_linear - Set implicit GeneratorParams. Required because Halide can't imply them from elsewhere.
add_halide_library(srgb_to_linear FROM color.generators
        GENERATOR srgb_to_linear
        AUTOSCHEDULER Halide::Adams2019
        PARAMS auto_schedule=true srgb.type=float32 srgb.dim=3
        SCHEDULE srgb_to_linear_schedule)

# Install
file(GLOB GENERATED_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/*.h")
install(FILES ${GENERATED_HEADERS} DESTINATION ${CMAKE_SOURCE_DIR}/dist/include)
install(TARGETS srgb_to_linear srgb_to_linear.runtime DESTINATION ${CMAKE_SOURCE_DIR}/dist/lib)