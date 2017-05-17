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
 * [libRocket](http://librocket.com/)
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

## Settings
You can create a settings.ini file by running the program once. Then you can simply change the relevant data. Alternatively, copy this one:
```
[Window]
resx=1366
resy=768
fov=90.000000
[Renderer]
graphics=OpenGL
reflections=1
shadows=1
[Game]
defaultmap=../scenes/sponza.json
```

## Assets
### Maps
To add new maps, simply create a new JSON file. You can change map files from settings.ini, under ```[GAME]```, just set the path of the ```defaultmap```. We have a few samples for you to choose from but you can make your own. Later, binary files will automatically be created.

To author your own, you need a root component. This needs an array of objects called ```entities```. For optimization, please create a field before it  called ```numentities```. The same can be said for ```cubemaps``` and ```numcubemaps``` for cubemap probe reflections, though cubemaps simply are a list of positions.

Entities for now take a ```name``` string (unused for now), and a ```components``` array of objects. ```scale```, ```position```, and ```angles``` are all temporarily used in them but they should eventually be put in a seperate ```COMPONENT_POSITION``` component.

Each component has an componentType string property. It also has more features depending on the type.
 * ```COMPONENT_RENDER``` takes a ```path``` string path to the model (.obj, .fbx, etc) file.
 * ```COMPONENT_LIGHT_DIRECTIONAL``` takes ```castshadow``` boolean, ```color``` 3-array of floats, ```brightness``` float, and ```sunradius``` float.
 * ```COMPONENT_LIGHT_POINT``` takes ```castshadow``` boolean, ```color``` 3-array of floats, ```brightness``` float, and ```lightradius``` float.
 * ```COMPONENT_LIGHT_SPOT``` takes ```castshadow``` boolean, ```color``` 3-array of floats, and ```brightness```, ```lightradius```, ```innerangle```, and ```outerangle``` floats.
 * ```COMPONENT_LIGHT_TERRAIN``` takes ```width```,  ```height```,  ```length``` floats, a  ```patches``` int for the number of patches in each axis, and  ```heightmap``` for the path to the heightmap.

### Sponza
You can download the Sponza assets at:
 * [Alexandre Pestana's site for the PBR Textures](http://www.alexandre-pestana.com/pbr-textures-sponza/)
 * [CryTek's site for the sponza_obj.rar file.](http://www.crytek.com/cryengine/cryengine3/downloads)
We are not affiliated with either of these websites.

## Contributors
The Grindstone Engine is a product of InOrdinate Studios. It's being worked on by Karim Abdel Hamid. Special thanks to Skyus, Muhammad Nael, "Don" Omar El-Behady, and "Loufis" Youssef Rehab.

## License
This is closed source, at least for now. Only using repository for internal usage! Do not leak or distribute the code.