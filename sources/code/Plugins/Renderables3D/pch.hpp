#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef RENDERABLES_3D_DLL
        #define RENDERABLES_3D_API __declspec(dllexport)
    #else
        #define RENDERABLES_3D_API __declspec(dllimport)
    #endif
#else
    #define RENDERABLES_3D_API
#endif