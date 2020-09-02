#pragma once

#ifdef _WIN32
    #ifdef CAMERA_DLL
        #define CAMERA_API __declspec(dllexport)
    #else
        #define CAMERA_API __declspec(dllimport)
    #endif
#else
    #define CAMERA_DLL
#endif