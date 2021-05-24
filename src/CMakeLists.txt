# color.cpp generators
add_executable(color.generators color.cpp)
target_link_libraries(color.generators
        PRIVATE Halide::Generator)

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