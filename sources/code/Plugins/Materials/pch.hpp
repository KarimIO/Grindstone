#pragma once

#ifdef _WIN32
    #ifdef RENDERER_MATERIAL_DLL
        #define RENDERER_MATERIAL_API __declspec(dllexport)
    #else
        #define RENDERER_MATERIAL_API __declspec(dllimport)
    #endif
#else
    #define RENDERER_MATERIAL_API
#endif