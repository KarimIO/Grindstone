#ifndef _AUDIO_H
#define _AUDIO_H

#include "SoundFileSDL.h"
#include <vector>

#ifdef _WIN32
	#include <Windows.h>

	#ifdef AUDIO_DLL
		#define AUDIO_EXPORT_CLASS __declspec(dllexport) 
		#define AUDIO_EXPORT __declspec(dllexport) 
	#else
		#define AUDIO_EXPORT_CLASS __declspec(dllimport) 
		#define AUDIO_EXPORT __declspec(dllimport) 
	#endif
#else
	#define AUDIO_EXPORT_CLASS
	#define AUDIO_EXPORT extern "C"
#endif

class AudioSystem {
private:
	std::vector<SoundFileSDL *> soundFiles;
public:
	virtual bool Initialize();
	virtual SoundFile *LoadSound(const char *);
	virtual void Shutdown();
};

extern "C" {
	AUDIO_EXPORT AudioSystem *createAudio();
};

#endif