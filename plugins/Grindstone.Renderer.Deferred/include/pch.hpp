#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef RENDERER_DEFERRED_DLL
        #define RENDERER_DEFERRED_EXPORT __declspec(dllexport)
    #else
        #define RENDERER_DEFERRED_EXPORT __declspec(dllimport)
    #endif
#else
    #define RENDERER_DEFERRED_EXPORT
#endif
