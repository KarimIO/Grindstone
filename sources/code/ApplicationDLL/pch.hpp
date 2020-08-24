#pragma once

#ifdef _WIN32
    #ifdef APP_DLL
        #define APP_API __declspec(dllexport)
    #else
        #define APP_API __declspec(dllimport)
    #endif
#else
    #define APP_API 
#endif