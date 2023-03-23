#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef ANIMATOR_EXPORT
        #define ANIMATOR_EXPORT __declspec(dllexport)
    #else
        #define ANIMATOR_EXPORT __declspec(dllimport)
    #endif
#else
    #define ANIMATOR_EXPORT
#endif
