#pragma once

#ifdef _WIN32
    #ifdef GRAPHICS_OPENGL
        #define GRAPHICS_OPENGL_API __declspec(dllexport)
    #else
        #define GRAPHICS_OPENGL_API __declspec(dllimport)
    #endif
#else
    #define GRAPHICS_OPENGL_API
#endif