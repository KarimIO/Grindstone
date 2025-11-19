#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef RENDER_GRAPH_DLL
        #define RENDER_GRAPH_EXPORT __declspec(dllexport)
    #else
        #define RENDER_GRAPH_EXPORT __declspec(dllimport)
    #endif
#else
    #define RENDER_GRAPH_EXPORT
#endif
