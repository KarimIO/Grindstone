# Sources
All the source files. include, lib, and obj (intermediate files compiled in any non-Windows system) can be placed here but are ignored by git. If any module becomes extensive enough, seperate readme.md should be created for them.

## AudioModule
AudioModule is a dynamically linked module for SDL Audio. It is deprecated and needs rework.
Libraries Required: [SDL](https://www.libsdl.org/).

## Converter
Converter is a standalone executable tool to allow you to convert any model file to the Grindstone proprietary formats, for both models and materials. Soon ConverterDDS will be integrated into it.
Libraries Required: [STB](https://github.com/nothings/stb/), [Assimp](http://www.assimp.org/).

### ConverterDDS
ConverterDDS is a temporary tool to work on DDS texture conversion before integrating it into Converter. Simply launch it and it will allow you to input an image to convert.
Libraries Required: [STB](https://github.com/nothings/stb/).

## Engine
The main engine and executable. Contains structural code including game, scene, and geometry, as well as most of the other bits of code that don't belong to any specific module. Use ```$ make Engine``` to compile into /bin/Grindstone if you're using make on Linux.
Libraries Required: RapidJson, [Bullet](http://bulletphysics.org/), [GLM](http://glm.g-truc.net/), and [STB](https://github.com/nothings/stb/).
Upcoming libraries: [libRocket](http://librocket.com/).

## GraphicsModules
This will contain all the files related to each individual Graphics Library. For now, it will be OpenGL specific, but when DirectX and Vulkan are added, it will be seperated. Use ```$ make Graphics``` to compile into /bin/graphics.so if you're using make on Linux.

### Classes
 * RenderPass
 * Framebuffer
 * GraphicsPipeline
 * Texture
 * GraphicsWrapper
 * VertexBuffer
 * IndexBuffer
 * VertexArrayObject
 * UniformBuffer

### GraphicsCommon
Contains general header data, included by DX, OGL, and VK.

### GraphicsDirectX
Libraries Required: DirectX (Download [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)).

### GraphicsOpenGL
Libraries Required: [GL3W](https://github.com/skaslev/gl3w).

### GraphicsVulkan
Libraries Required: [Lunar API](https://vulkan.lunarg.com/sdk/home).

### WindowModule
Contains window general implementations to be used in the other projects. Both to create the window and to extract events from it. Any graphics language specific code is placed in other files. These files are compiled with the main graphics library modules.

#### These files contain implementations of:
 * Initializer Functions
 * int TranslateKey(int Keycode) - Translates Keycode from X11/GLFW/Win32 to Grindstone Keycodes
 * HandleEvents() - Calls InputSystem Events from the windowing
 * ResetCursor() - Sets cursor position to center screen
 * SetCursor(int x, int y) - Sets Cursor to x,y
 * GetCursor(int &x, int &y) - Sets x,y to cursor position

#### Files:
 * win32Window.cpp
    Libraries Required: Win32 Windows API.
 * x11Window.cpp
    Libraries Required: X11.
 * glfwWindow.cpp
    Libraries Required: [GLFW](http://www.glfw.org/).

    There are plans for Cocoa integration, but we're using GLFW instead for now.