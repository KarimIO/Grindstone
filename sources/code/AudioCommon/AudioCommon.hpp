#ifndef _AUDIO_H
#define _AUDIO_H

#ifdef _WIN32
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

#include "SoundBuffer.hpp"
#include "SoundSource.hpp"

class AudioWrapper {
public:
    virtual ~AudioWrapper() = 0;

    virtual SoundBuffer* CreateBuffer(SoundBufferCreateInfo create_info) = 0;
    virtual SoundSource* CreateSource() = 0;
    virtual SoundSource* CreateSource(SoundSourceCreateInfo create_info) = 0;

	virtual void SetListenerPosition(float x, float y, float z) = 0;
	virtual void SetListenerVelocity(float x, float y, float z) = 0;
	virtual void SetListenerOrientation(float targ_x, float targ_y, float targ_z, float up_x, float up_y, float up_z) = 0;
};

extern "C" AUDIO_EXPORT AudioWrapper* createAudio();
extern "C" AUDIO_EXPORT void deleteAudio(AudioWrapper *ptr);

#endif