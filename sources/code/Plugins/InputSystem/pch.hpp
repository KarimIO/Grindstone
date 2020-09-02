#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef INPUT_SYSTEM_DLL
        #define INPUT_SYSTEM_API __declspec(dllexport)
    #else
        #define INPUT_SYSTEM_API __declspec(dllimport)
    #endif
#else
    #define INPUT_SYSTEM_API
#endif