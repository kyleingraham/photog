# Find doctest
find_package(doctest 2 REQUIRED)

# Find JPEG and PNG libraries
find_package(JPEG 90 REQUIRED)
find_package(PNG 1 REQUIRED)

# Test executable
add_executable(tests tests.cpp)
target_link_libraries(tests
        PRIVATE
        color
        color_halide_libraries_bundle
        color_utils
        doctest::doctest
        Halide::Tools
        ${JPEG_LIBRARIES}
        ${PNG_LIBRARIES})

include(doctest) # enables doctest_discover_tests

doctest_discover_tests(tests)

## Copy non-source test files to test executable working directory
## This approach uses more disk space but prevents modification to test files
file(COPY ${CMAKE_SOURCE_DIR}/test/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})