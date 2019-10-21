# The Grindstone Engine
_A complete game engine and toolkit._

## Synopsis
The Grindstone Engine is an afforadable, extensible engine that contains all tools necessary to create modern games. It was created to compete with the huge amount of tools used today.

## Building
### All OSs
The following dependencies are already included in sources/deps/:
 * [GL3W](https://github.com/skaslev/gl3w)
 * [stb_image.h](https://github.com/nothings/stb)

### Windows - Visual Studio
Install the following Windows dependencies.
 * [Assimp](http://www.assimp.org/)
 * [Bullet](http://bulletphysics.org/)
 * [CMake](https://cmake.org/)
 * [libRocket](http://librocket.com/)
 * [OpenAL](https://www.openal.org/)
 * [SDL](https://www.libsdl.org/)
Optional Graphics-Specific dependencies:
 * [VULKAN: Lunar API](https://vulkan.lunarg.com/sdk/home)
 * [DIRECTX: Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)

All you need to do is run CMake and build the projects. Change the working directory of the engine project to [PROJECT_DIRECTORY]/bin/ if you want to debug it. 

### Fedora / OpenSUSE / Red Hat
You must first update and get all the following dependencies:
```
$ sudo dnf update 
$ sudo dnf install gcc-c++ make assimp-devel xorg-x11-apps mesa-libGL-devel cmake
```

### Ubuntu / Debian
You must first update and get all the following dependencies:
```
$ sudo apt update 
$ sudo apt install g++ make libassimp-dev libgl1-mesa-dev xorg xorg-dev cmake libopenal-dev mono-devel
```

To build libRocket, download the repository, enter the ```Build``` folder, and call the following:
```
cmake -DBUILD_SAMPLES=off -DBUILD_LUA_BINDINGS=off -DCMAKE_BUILD_TYPE=Debug -DROCKET_DEBUG=on -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_INSTALL_PREFIX=$HOME/local
make
make install
```

To build bullet, download the repository, enter the main folder, and call the following:
```
mkdir Build
cd Build
cmake .. -G "Unix Makefiles" -DINSTALL_LIBS=ON
make -j4
sudo make install
```

To build the actual project, use the makefile. To compile all files, just run ```make```

To compile just the game engine: ```make Engine```

To clean output: ``` make clean```

To run, go to the ```./bin``` folder, and use ``` ./Grindstone```

### MacOS
You need to install the [Homebrew](https://brew.sh/) macOS package manager using Terminal:
```
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
```

To install all the required dependencies using Homebrew:
```
brew install gcc48 glm glfw3 glew cmake assimp python freetype rapidjson
```

To install the X.Org window system on macOS, install [XQuartz](https://www.xquartz.org/releases/XQuartz-2.7.11.html), and create the following symlinks using Terminal:
```
ln -s /opt/X11/include/X11 /usr/local/include/X11
ln -s /opt/X11/include/GL /usr/local/include/GL
```

To install bullet:
```
brew install bullet
```

To install and build libRocket:
```
git clone https://github.com/libRocket/libRocket.git
cd libRocket
cd Build
cmake -DBUILD_SAMPLES=off -DBUILD_LUA_BINDINGS=off -DCMAKE_BUILD_TYPE=Debug -DROCKET_DEBUG=on -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_INSTALL_PREFIX=$HOME/local
make
make install
``` 

To build the actual project, use the makefile. To compile all files, just run: ```make```

To compile just the game engine: ```make Engine```

To clean output: ``` make clean```

To run, go to the ```./bin``` folder, and use ``` ./Grindstone```

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
### File Formats
All file formats are discussed [here.](formats.md)

### Sponza
You can download a level from here: [Gist](https://gist.github.com/KarimIO/2dc004cac2bf0dd56e7c4f76975eb856)
You can download the Sponza assets converted to Grindstone formats here: [Google Drive](https://drive.google.com/open?id=1ESJ7S98VKbvcpkhTM3jJPWlnUiBLrgV0)

You can download the Sponza original assets at:
 * [Alexandre Pestana's site for the PBR Textures](http://www.alexandre-pestana.com/pbr-textures-sponza/)
 * [CryTek's site for the sponza_obj.rar file.](http://www.crytek.com/cryengine/cryengine3/downloads)
We are not affiliated with either of these websites.

## Contributors
The Grindstone Engine is a product of InOrdinate Studios. It's being worked on by Karim Abdel Hamid. Special thanks to Skyus, Muhammad Nael, "Don" Omar El-Behady, and "Loufis" Youssef Rehab.

## License
This is closed source, at least for now. Only using repository for internal usage! Do not leak or distribute the code.