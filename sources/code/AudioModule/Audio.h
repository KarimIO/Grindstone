#ifndef _AUDIO_H
#define _AUDIO_H

#include <SDL.h>
#include <SDL_mixer.h>

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
	Mix_Chunk* _sample[2];
public:
	virtual bool Initialize();
	virtual void Play(int id);
	virtual void Shutdown();
};

extern "C" {
	AUDIO_EXPORT AudioSystem *createAudio();
};

#endif