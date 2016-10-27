#The Grindstone Engine
_A complete game engine and toolkit._

## Synopsis
The Grindstone Engine is an afforadable, extensible engine that contains all tools necessary to create modern games. It was created to compete with the huge amount of tools used today.

## Building
### All OSs
Install all the following dependencies regardless of OS
 * [GL3W](https://github.com/skaslev/gl3w)
 * [stb_image.h](https://github.com/nothings/stb)

### Windows
Install the following Windows dependencies.
 * [Assimp](http://www.assimp.org/)
 * [Windows SDK for DirectX](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)

Build using the following libs:
> assimp-vcxxx-mtd.lib, opengl32.lib 

To build on windows, create a solution outside of the main folder with three projects. Ensure all three export to ```/Grindstone/bin/```.

Make sure that two projects are of type .dll, and compile to ```window.dll``` and ```graphics.dll```.

Make the final project use ```graphics.lib;window.lib``` as well as include the source folders in the additional include directory input. This should compile to ```Grindstone.exe```
 
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