#pragma once

#ifdef _WIN32
    #ifdef AUDIO_OPENAL
        #define AUDIO_OPENAL_API __declspec(dllexport)
    #else
        #define AUDIO_OPENAL_API __declspec(dllimport)
    #endif
#else
    #define AUDIO_OPENAL_API
#endif
