#pragma once

#ifdef _WIN32
    #ifdef GRAPHICS_VULKAN
        #define GRAPHICS_VULKAN_API __declspec(dllexport)
    #else
        #define GRAPHICS_VULKAN_API __declspec(dllimport)
    #endif
#else
    #define GRAPHICS_VULKAN_API
#endif