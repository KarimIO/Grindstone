<div align="center">

![Logo](./readmeImages/grindstoneLogo.png#gh-dark-mode-only)
![Logo](./readmeImages/grindstoneLogoLightMode.png#gh-light-mode-only)

# The Grindstone Engine

The Grindstone Engine is an extensible engine that contains all tools to create modern games. It includes a C# Scripting Engine, IBL and PBR-based graphics, a visual editor, and more.

</div>

![Runtime](readmeImages/grindstone.jpg)

## Building
 - Install [vcpkg](https://vcpkg.io/en/getting-started.html) and [CMake](https://cmake.org/install/).
 - Configure the location of CMAKE_TOOLCHAIN_FILE or set the environment variable VCPKG_PATH.
 - Install Mono Project
 - Copy mono-2.0-sgen.dll to the bin folder
 - Just run Cmake!

## Example Project
Run `ApplicationExecutable.exe` with `-projectpath "Path\To\Project"` to run a project.
[Grindstone Sandbox](https://github.com/KarimIO/Grindstone-Sandbox).

![Editor](readmeImages/editor.jpg)

## License
This is closed source, at least for now. Only using repository for internal usage! Do not leak or distribute the code.
