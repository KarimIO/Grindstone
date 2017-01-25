#include "Audio.h"

#include <iostream>
#include <Windows.h>

#define NUM_WAVEFORMS 2
const char* _waveFileNames[] =
{
	"../sounds/kickdrum.wav",
	"../sounds/snaredrum.wav",
};

// Initializes the application data
bool AudioSystem::Initialize(void) {
	memset(_sample, 0, sizeof(Mix_Chunk*) * 2);

	// Set up the audio stream
	int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512); // 44100
	if (result < 0) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		return false;
	}

	result = Mix_AllocateChannels(4);
	if (result < 0)	{
		fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
		return false;
	}

	// Load waveforms
	for (int i = 0; i < NUM_WAVEFORMS; i++) {
		_sample[i] = Mix_LoadWAV(_waveFileNames[i]);
		if (_sample[i] == NULL)	{
			fprintf(stderr, "Unable to load wave file: %s\n", _waveFileNames[i]);
		}
	}

	return true;
}

void AudioSystem::Play(int id) {
	Mix_PlayChannel(-1, _sample[id], 0);
}

void AudioSystem::Shutdown() {
	for (int i = 0; i < NUM_WAVEFORMS; i++)
	{
		Mix_FreeChunk(_sample[i]);
	}

	Mix_CloseAudio();
	SDL_Quit();
}

AUDIO_EXPORT AudioSystem *createAudio() {
	return new AudioSystem;
}