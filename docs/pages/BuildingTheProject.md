Building The Project {#BuildingTheProject}
============

## Automatic Building

Run either `scripts/setup-win.bat` on Windows, or `scripts/setup-unix.sh` on Linux or Mac.

## Manual Building

Here is how to set up the project manually:

 - Install some prerequisites:
    - [vcpkg](https://vcpkg.io/en/getting-started.html)
    - [Git](https://git-scm.com/download/win)
    - [CMake](https://cmake.org/install/).
 - Configure the location of `CMAKE_TOOLCHAIN_FILE` to your `vcpkg/scripts/buildsystems/vcpkg.cmake` or set the environment variable `VCPKG_PATH` to your vcpkg path.
 - Run Cmake through one of two methods:
    - In the Cmake GUI, press "Configure" and "Generate". You may have to set the generator, and you can set it to your build tool - Visual Studio on Windows, Makefiles on Linux/MacOS.
    - In command prompt, run: `cmake.exe -DCMAKE_TOOLCHAIN_FILE="!VCPKG_PATH!/scripts/buildsystems/vcpkg.cmake" ./`
 - Build the project - via Visual Studio on Windows and Makefiles on Linux/MacOS.
