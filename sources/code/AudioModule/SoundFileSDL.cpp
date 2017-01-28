#include "SoundFileSDL.h"

void SoundFileSDL::Play() {
	Mix_PlayChannel(0, sound, 0);
}

void SoundFileSDL::PlayLoop(int id) {
	Mix_PlayChannel(0, sound, id);
}

void SoundFileSDL::Stop() {
	Mix_Pause(0);
}

SoundFileSDL::~SoundFileSDL() {
	Mix_FreeChunk(sound);
}
