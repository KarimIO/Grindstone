#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef RENDERPATH_DEFERRED_DLL
        #define RENDERPATH_DEFERRED_API __declspec(dllexport)
    #else
        #define RENDERPATH_DEFERRED_API __declspec(dllimport)
    #endif
#else
    #define RENDERPATH_DEFERRED_API
#endif