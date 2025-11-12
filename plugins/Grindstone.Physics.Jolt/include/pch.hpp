#pragma once

#ifdef _WIN32
    #ifdef PHYSICS_DLL
        #define JOLT_PHYSICS_EXPORT __declspec(dllexport)
    #else
        #define JOLT_PHYSICS_EXPORT __declspec(dllimport)
    #endif
#else
    #define JOLT_PHYSICS_EXPORT
#endif
