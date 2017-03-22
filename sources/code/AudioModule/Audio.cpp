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
bool AudioSystem::Initialize() {
	// Set up the audio stream
	int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024); // 44100
	if (result < 0) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		return nullptr;
	}

	result = Mix_AllocateChannels(4);
	if (result < 0) {
		fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
		return nullptr;
	}
}

SoundFile *AudioSystem::LoadSound(const char *path) {
	Mix_Chunk* _sample = NULL;
	memset(&_sample, 0, sizeof(_sample));

	// Load waveforms
	_sample = Mix_LoadWAV(path);
	if (_sample == NULL) {
		fprintf(stderr, "Unable to load wave file: %s\n", path);
	}

	soundFiles.push_back(new SoundFileSDL());
	soundFiles.back()->sound = _sample;

	return (SoundFile *)soundFiles.back();
}

void AudioSystem::SetChannelVolume(int channel, int dist, int ang) {
	Mix_SetPanning(MIX_CHANNEL_POST, dist, ang);
}

void AudioSystem::Shutdown() {
	for (size_t i = 0; i < soundFiles.size(); i++)
		delete soundFiles[i];

	Mix_CloseAudio();
	SDL_Quit();
}

AUDIO_EXPORT AudioSystem *createAudio() {
	return new AudioSystem;
}