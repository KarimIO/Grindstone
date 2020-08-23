#pragma once

#ifdef _WIN32
    #ifdef APP_DLL
        #define APP_EXPORT __declspec(dllexport)
    #else
        #define APP_EXPORT __declspec(dllimport)
    #endif
#else
    #define APP_EXPORT 
#endif