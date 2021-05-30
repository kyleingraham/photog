# Find doctest
if (APPLE)
    find_package(doctest REQUIRED)
endif ()

# Test executable
add_executable(tests tests.cpp)

target_link_libraries(tests
        PRIVATE
        doctest::doctest
        Halide::Tools
        color) # linking any add_halide_library target brings in its headers and libraries

## Linking for JPEG/PNG libraries is different between OS's
if (APPLE)
    target_link_libraries(tests PRIVATE ${JPEG_LIBRARIES})
    target_link_libraries(tests PRIVATE ${PNG_LIBRARIES})
endif ()

include(doctest) # enables doctest_discover_tests

doctest_discover_tests(tests)

## Copy non-source test files to test executable working directory
## This approach uses more disk space but prevents modification to test files
file(COPY ${CMAKE_SOURCE_DIR}/test/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(benchmarks)