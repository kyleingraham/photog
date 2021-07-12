cmake_minimum_required(VERSION 3.19)
project(photog
        VERSION 0.0.1) # Provides access to photog_VERSION and its variants

include(CTest)

# Setup language settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Find doctest
find_package(doctest 2 REQUIRED)

# Find Halide
if (APPLE)
    # Prevents latching onto JPEG/PNG libraries vendored within frameworks
    set(CMAKE_FIND_FRAMEWORK LAST)
endif ()
find_package(Halide 12 REQUIRED)

# Global settings
## Extent estimates for generators
if (DEFINED PHOTOG_IMAGE_WIDTH_ESTIMATE)
    set(photog_IMAGE_WIDTH_ESTIMATE ${PHOTOG_IMAGE_WIDTH_ESTIMATE})
elseif (DEFINED ENV{PHOTOG_IMAGE_WIDTH_ESTIMATE})
    set(photog_IMAGE_WIDTH_ESTIMATE $ENV{PHOTOG_IMAGE_WIDTH_ESTIMATE})
else ()
    set(photog_IMAGE_WIDTH_ESTIMATE 500)
endif ()

if (DEFINED PHOTOG_IMAGE_HEIGHT_ESTIMATE)
    set(photog_IMAGE_HEIGHT_ESTIMATE ${PHOTOG_IMAGE_HEIGHT_ESTIMATE})
elseif (DEFINED ENV{PHOTOG_IMAGE_HEIGHT_ESTIMATE})
    set(photog_IMAGE_HEIGHT_ESTIMATE $ENV{PHOTOG_IMAGE_HEIGHT_ESTIMATE})
else ()
    set(photog_IMAGE_HEIGHT_ESTIMATE 500)
endif ()

## Image layout - valid options: interleaved, planar
if (DEFINED PHOTOG_IMAGE_LAYOUT)
    set(photog_IMAGE_LAYOUT ${PHOTOG_IMAGE_LAYOUT})
elseif (DEFINED ENV{PHOTOG_IMAGE_LAYOUT})
    set(photog_IMAGE_LAYOUT $ENV{PHOTOG_IMAGE_LAYOUT})
else ()
    set(photog_IMAGE_LAYOUT "planar")
endif ()

message(STATUS "photog image layout:             ${photog_IMAGE_LAYOUT}")
message(STATUS "photog image width estimate:     ${photog_IMAGE_WIDTH_ESTIMATE}px")
message(STATUS "photog image height estimate:    ${photog_IMAGE_HEIGHT_ESTIMATE}px")
# TODO: Add CMakePresets.json file.
# TODO: Can we provide a useful message if a setting is not valid?
# TODO: Add CMake function for loading CLI/environment arguments.
# TODO: Allow user to specify variables from their project that includes photog.

add_subdirectory(src)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif ()