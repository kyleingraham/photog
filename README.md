# photog
Computational photography library built on [Halide](https://halide-lang.org/).

photog takes advantage of Halide's unique strengths...
- Separation of algorithms from their mapping onto resources (scheduling).
- Auto-scheduling based on hardware features, image size, and image memory 
  layout.
- Auto-scheduling support for many CPU platforms.
- The ability to produce extremely performant algorithms while just focusing on
  the algorithms!
  
...to produce a computational photography library that provides performant 
functions tuned to the hardware and images you wish to run them on.

## Configuration
photog will use Halide's auto-scheduling to compile versions of its functions
that are tuned for the platform you are compiling on. Auto-scheduling is also 
dependent on image memory layout and expected dimensions. photog provides the
following hooks to specify that information:

CMake Option | Environment Variable | Default | Description
-------------|----------------------|---------|------------
`PHOTOG_IMAGE_LAYOUT` | `PHOTOG_IMAGE_LAYOUT` | planar | Valid options are `planar` and `interleaved`. Planar images are contiguous in channels while interleaved images are contiguous in pixels. Best performance is achieved with planar images.
`PHOTOG_IMAGE_WIDTH_ESTIMATE` | `PHOTOG_IMAGE_WIDTH_ESTIMATE`| 500 | Expected width in pixels of images to be processed.
`PHOTOG_IMAGE_HEIGHT_ESTIMATE` | `PHOTOG_IMAGE_HEIGHT_ESTIMATE`| 500 | Expected height in pixels of images to be processed.

Image dimension estimates provide a guideline for scheduling and in most cases 
do not exclude smaller or larger images.

## Building
### Dependencies
photog depends on the following libraries:
- Halide (version 12)
- doctest (version 2)
- libjpeg (only if building tests)
- libpng (only if building tests)

You will also need a C++17 compiler.

### Build/Install Commands
```shell
$ git clone https://github.com/kyleingraham/photog.git
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DPHOTOG_IMAGE_LAYOUT=planar \
        -DPHOTOG_IMAGE_WIDTH_ESTIMATE=500 \
        -DPHOTOG_IMAGE_HEIGHT_ESTIMATE=500 \
        -G "<Your choice of generator>" \
        ./photog/
$ cmake --build ./photog/cmake-build-release --target install
```
On Windows, CMake's Ninja generator will not work. Use a 
[Visual Studio generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) 
instead.

### Usage via CMake
To use photog in your CMake project add the following command to your `CMakeLists.txt`:
```cmake
find_package(photog)
```
The following variables and targets will be made available:
#### Set Variables
Variable | Description
---------|------------
`photog_VERSION` | Full version string
`photog_VERSION_MAJOR` | Major version string
`photog_VERSION_MINOR` | Minor version string
`photog_VERSION_PATCH` | Patch version string
`photog_VERSION_TWEAK` | Tweak version string
`photog_TARGET` | Halide target triple (`arch-bits-os`) used to compile photog
`photog_IMAGE_LAYOUT` | Image layout (`planar` or `interleaved`) used to complile photog
`photog_IMAGE_WIDTH_ESTIMATE` | Image width estimate in pixels used to compile photog
`photog_IMAGE_HEIGHT_ESTIMATE` | Image height estimate in pixels used to compile photog

#### Imported Targets
Target | Description
-------|------------
`photog::color` | Makes available the `photog/color.h` header containing functions for manipulating image colors/color spaces.

## Available Functions
Defined in header `photog/color.h` (`photog::color` target):
```c++
/** Chromatically adapt RGB input from the estimated source illuminant of the
 * input image to the given destination illuminant.
 *
 * The source illuminant is estimated using the gray-world method.
 */
void photog_chromadapt(float *input, int width, int height,
                       PhotogWorkingSpace working_space,
                       PhotogChromadaptMethod chromadapt_method,
                       PhotogIlluminant dest_illuminant, float *output);

/** Chromatically adapt RGB input from the given source illuminant to the given
 * destination illuminant.
 *
 * Here both the source and destination tristimulus values must be supplied by
 * the user hence the "_diy" prefix.
 */
void photog_chromadapt_diy(float *input, int width, int height,
                           float *source_tristimulus,
                           PhotogWorkingSpace working_space,
                           PhotogChromadaptMethod chromadapt_method,
                           float *dest_tristimulus, float *output);
```
Detailed function descriptions are available in their respective headers.

## Missing Functionality
- Easy cross-compilation
- GPU support