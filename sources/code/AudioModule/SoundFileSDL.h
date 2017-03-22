#ifndef _SOUNDFILE_H
#define _SOUNDFILE_H

#include "SoundFile.h"

#include <SDL.h>
#include <SDL_mixer.h>

class SoundFileSDL : public SoundFile {
public:
	Mix_Chunk* sound;
	virtual void Play();
	virtual void PlayLoop(int id);
	virtual void Stop();
	~SoundFileSDL();
};

#endif