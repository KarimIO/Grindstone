#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include "al.h"
#include "alc.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "AudioClipImporter.hpp"
#include <EngineCore/Logger.hpp>

using namespace Grindstone::Audio;

AudioClipImporter::AudioClipImporter(EngineCore* engineCore) {
	this->engineCore = engineCore;
}

static bool LoadAudioClip(Grindstone::Audio::AudioClipAsset& audioClipAsset) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	audioClipAsset.buffer = 0;

	std::filesystem::path path = engineCore.GetAssetPath(audioClipAsset.uuid.ToString());
	std::string pathString = path.string();
	const char* pathCstr = pathString.c_str();
	if (!std::filesystem::exists(path)) {
		GPRINT_ERROR_V(Grindstone::LogSource::Audio, "AudioClipImporter::LoadFromPath - Could not find file:", pathString.c_str());
		audioClipAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Missing;
		return nullptr;
	}

	drwav wav;
	if (!drwav_init_file(&wav, pathCstr, nullptr)) {
		GPRINT_ERROR_V(Grindstone::LogSource::Audio, "AudioClipImporter::LoadFromPath - Failed to load file:", pathString.c_str());
		audioClipAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
		return nullptr;
	}

	drwav_uint16 channelCount = wav.channels;
	drwav_uint16 bitsPerSample = wav.bitsPerSample;
	drwav_uint32 sampleRate = wav.sampleRate;

	size_t fileSize = wav.bytesRemaining;
	char* memoryBuffer = (char*)malloc(fileSize);
	size_t numberOfSamplesActuallyDecoded = drwav_read_raw(&wav, fileSize, memoryBuffer);

	ALuint buffer;
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
		GPRINT_ERROR_V(Grindstone::LogSource::Audio, "ERROR: unrecognised wave format: {} channels, {} bits per sample.", channelCount, bitsPerSample);
		audioClipAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
		return false;
	}

	alBufferData(buffer, format, memoryBuffer, static_cast<ALsizei>(fileSize), sampleRate);

	audioClipAsset.bitsPerSample = bitsPerSample;
	audioClipAsset.buffer = buffer;
	audioClipAsset.channelCount = channelCount;
	audioClipAsset.sampleRate = sampleRate;
	audioClipAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Ready;

	drwav_free(memoryBuffer, nullptr);
	drwav_uninit(&wav);

	return true;
}

void* AudioClipImporter::LoadAsset(Uuid uuid) {
	auto& assetIterator = assets.emplace(
		uuid,
		AudioClipAsset(uuid)
	);

	Grindstone::Audio::AudioClipAsset& audioClipAsset = assetIterator.first->second;
	audioClipAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (LoadAudioClip(audioClipAsset)) {
		return nullptr;
	}

	return &audioClipAsset;
}

void AudioClipImporter::QueueReloadAsset(Uuid uuid) {
	auto& audioClipInMap = assets.find(uuid);
	if (audioClipInMap == assets.end()) {
		return;
	}

	Grindstone::Audio::AudioClipAsset& audioClipAsset = audioClipInMap->second;

	if (audioClipAsset.buffer != 0) {
		alDeleteBuffers(1, &audioClipInMap->second.buffer);
	}

	audioClipAsset.assetLoadStatus = AssetLoadStatus::Loading;
	LoadAudioClip(audioClipAsset);
}

AudioClipImporter::~AudioClipImporter() {
	for (auto& asset : assets) {
		if (asset.second.buffer != 0) {
			alDeleteBuffers(1, &asset.second.buffer);
		}
	}

	assets.clear();
}
