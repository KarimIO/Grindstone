#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#ifdef _WIN32
    #ifdef EDITOR_MATERIAL_IMPORTER_DLL
        #define EDITOR_MATERIAL_IMPORTER_EXPORT __declspec(dllexport)
    #else
        #define EDITOR_MATERIAL_IMPORTER_EXPORT __declspec(dllimport)
    #endif
#else
    #define EDITOR_MATERIAL_IMPORTER_EXPORT
#endif
