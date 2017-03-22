#Sources
All the source files. include, lib, and obj (intermediate files compiled in any non-Windows system) can be placed here but are ignored by git. If any module becomes extensive enough, seperate readme.md should be created for them.

##Engine
The main engine and executable. Contains structural code including game, scene, and geometry, as well as most of the other bits of code that don't belong to any specific module. Use ```$ make Engine``` to compile into /bin/Grindstone if you're using make on Linux.

#WindowModule
This will contain all files specific to windowing, as well as limited input-related files. Use ```$ make Window``` to compile into /bin/window.so if you're using make on Linux.

#GraphicsModule
This will contain all the files related to each individual Graphics Library. For now, it will be OpenGL specific, but when DirectX and Vulkan are added, it will be seperated. Use ```$ make Graphics``` to compile into /bin/graphics.so if you're using make on Linux.