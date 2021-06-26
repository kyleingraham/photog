cmake_minimum_required(VERSION 3.19)
project(photog)

include(CTest)

# Setup language settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Find doctest
find_package(doctest REQUIRED)

# Find Halide
if (APPLE)
    # Prevents latching onto JPEG/PNG libraries vendored within frameworks
    set(CMAKE_FIND_FRAMEWORK LAST)
endif ()
find_package(Halide REQUIRED)

add_subdirectory(src)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif ()