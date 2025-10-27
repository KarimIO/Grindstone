#pragma once

#ifdef _WIN32
    #ifdef AI_NAVMESH_DLL
        #define AI_NAVMESH_EXPORT __declspec(dllexport)
    #else
        #define AI_NAVMESH_EXPORT __declspec(dllimport)
    #endif
#else
    #define AI_NAVMESH_EXPORT
#endif
