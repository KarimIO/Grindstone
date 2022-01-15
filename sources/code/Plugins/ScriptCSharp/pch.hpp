#pragma once

#ifdef _WIN32
    #ifdef CSHARP_DLL
        #define CSHARP_EXPORT __declspec(dllexport)
    #else
        #define CSHARP_EXPORT __declspec(dllimport)
    #endif
#else
    #define CSHARP_EXPORT 
#endif
