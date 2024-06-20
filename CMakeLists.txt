cmake_minimum_required(VERSION 3.28)
project(photog
        VERSION 0.0.1) # Provides access to photog_VERSION and its variants

include(CTest)
include(FetchContent)

# Setup language settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11)
FetchContent_MakeAvailable(doctest)
list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)

if (APPLE)
    # Prevents latching onto JPEG/PNG libraries vendored within frameworks
    set(CMAKE_FIND_FRAMEWORK LAST)

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        FetchContent_Declare(halide
                URL https://github.com/halide/Halide/releases/download/v17.0.1/Halide-17.0.1-x86-64-osx-52541176253e74467dabc42eeee63d9a62c199f6.tar.gz)
    else ()
        FetchContent_Declare(halide
                URL https://github.com/halide/Halide/releases/download/v17.0.1/Halide-17.0.1-arm-64-osx-52541176253e74467dabc42eeee63d9a62c199f6.tar.gz)
    endif ()
endif ()
FetchContent_MakeAvailable(halide)
list(APPEND CMAKE_PREFIX_PATH ${halide_SOURCE_DIR})
find_package(Halide 17 REQUIRED)

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

add_subdirectory(src)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif ()