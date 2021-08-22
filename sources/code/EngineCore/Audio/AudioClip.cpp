#include <filesystem>
#include <fstream>
#include <vector>
#include <bit>
#include "AL/al.h"
#include "AL/alc.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "EngineCore/Utils/Utilities.hpp"

#include "AudioClip.hpp"
using namespace Grindstone::Audio;

Clip::Clip(const char* path) {
	LoadWavFromPath(path);
}

void Clip::CreateOpenALBuffer(const char* bufferPtr, size_t bufferSize) {
	alGenBuffers(1, &buffer);

	ALenum format;
	if (channelCount == 1 && bitsPerSample == 8) {
		format = AL_FORMAT_MONO8;
	}
	else if (channelCount == 1 && bitsPerSample == 16) {
		format = AL_FORMAT_MONO16;
	}
	else if (channelCount == 2 && bitsPerSample == 8) {
		format = AL_FORMAT_STEREO8;
	}
	else if (channelCount == 2 && bitsPerSample == 16) {
		format = AL_FORMAT_STEREO16;
	}
	else {
		throw std::runtime_error(
			std::string("ERROR: unrecognised wave format: ") +
			std::to_string(channelCount) + " channels, " +
			std::to_string(bitsPerSample) + " bits per sample."
		);
	}

	alBufferData(buffer, format, bufferPtr, bufferSize, sampleRate);
}

ALuint Clip::GetOpenALBuffer() {
	return buffer;
}

void Clip::LoadFromPath(const char* path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error(std::string("Clip::LoadFromPath - Could not find file:") + path);
	}

	LoadWavFromPath(path);
}

void Clip::LoadWavFromPath(const char* path) {
	drwav* wav = drwav_open_file(path);
	if (wav == NULL) {
		throw std::runtime_error(std::string("Clip::LoadWavFromPath - Failed to load file:") + path);
	}

	channelCount = wav->channels;
	bitsPerSample= wav->bitsPerSample;
	sampleRate = wav->sampleRate;

	size_t fileSize = wav->totalSampleCount * wav->bytesPerSample;
	char* fileData = (char*)malloc(fileSize);
	drwav_read_raw(wav, fileSize, fileData);
	CreateOpenALBuffer(fileData, fileSize);

	drwav_free(fileData);
}

Clip::~Clip() {
	alDeleteBuffers(1, &buffer);
}