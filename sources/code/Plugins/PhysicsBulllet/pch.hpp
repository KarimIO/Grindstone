#pragma once

#ifdef _WIN32
    #ifdef PHYSICS_DLL
        #define BULLET_PHYSICS_EXPORT __declspec(dllexport)
    #else
        #define BULLET_PHYSICS_EXPORT __declspec(dllimport)
    #endif
#else
    #define BULLET_PHYSICS_EXPORT 
#endif
