#The Grindstone Engine
_A complete game engine and toolkit._

## Synopsis
The Grindstone Engine is an afforadable, extensible engine that contains all tools necessary to create modern games. It was created to compete with the huge amount of tools used today.

## Building
### All OSs
Install all the following dependencies regardless of OS
 * [GL3W](https://github.com/skaslev/gl3w)
 * [stb_image.h](https://github.com/nothings/stb)

### Windows - Visual Studio
Install the following Windows dependencies.
 * [Assimp](http://www.assimp.org/)
 * [Bullet](http://bulletphysics.org/)
 * [CMake](https://cmake.org/)
 * [GLM](http://glm.g-truc.net/)
 * [SDL](https://www.libsdl.org/)
 * [STB](https://github.com/nothings/stb/)
Optional Graphics-Specific dependencies:
 * [OPENGL: GL3W](https://github.com/skaslev/gl3w)
 * [VULKAN: Lunar API](https://vulkan.lunarg.com/sdk/home)
 * [DIRECTX: Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (Currently unused)

All you need to do is run CMake and build the projects. Change the working directory of the engine project to [PROJECT_DIRECTORY]/bin/ if you wanna debug it. 

### Debian
You must first update and get all the following dependencies:
```
$ sudo apt update 
$ sudo apt install g++ clang make libglm-dev libassimp-dev libgl1-mesa-dev xorg xorg-dev
```

To build, use the makefile. To compile all files, just run ```$ make```

To compile just the game engine: ```$ make Engine```

To clean output: ``` $ make clean```

To run, use ``` $ ./bin/Grindstone```

### Other OSs
Use an alternate OS, or pitch in and suggest changes!

## Contributors
The Grindstone Engine is a product of InOrdinate Studios. It's being worked on by Karim Abdel Hamid. Special thanks to Skyus, Muhammad Nael, "Don" Omar El-Behady, and "Loufis" Youssef Rehab.

## License
This is closed source, at least for now. Only using repository for internal usage! Do not leak or distribute the code.