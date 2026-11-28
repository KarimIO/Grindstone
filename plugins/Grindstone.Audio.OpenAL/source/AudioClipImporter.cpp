#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include "al.h"
#include "alc.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <Common/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Grindstone.Audio.OpenAL/include/AudioClipImporter.hpp>

using namespace Grindstone::Audio;

AudioClipImporter::AudioClipImporter() {}

static bool LoadAudioClip(Grindstone::Audio::AudioClipAsset& audioClipAsset) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	audioClipAsset.buffer = 0;

	
	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(Grindstone::AssetType::AudioClip, audioClipAsset.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_WARN(Grindstone::LogSource::EngineCore, "Unable to load audio: {}", audioClipAsset.uuid.ToString());
		audioClipAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Missing;
		return false;
	}

	audioClipAsset.name = result.displayName;

	drwav wav;
	if (!drwav_init_memory(&wav, result.buffer.Get(), result.buffer.GetCapacity(), nullptr)) {
		GPRINT_ERROR(Grindstone::LogSource::Audio, "AudioClipImporter::LoadFromPath - Failed to load file:", audioClipAsset.name.c_str());
		audioClipAsset.assetLoadStatus = Grindstone::AssetLoadStatus::Failed;
		return false;
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
		GPRINT_ERROR(Grindstone::LogSource::Audio, "ERROR: unrecognised wave format: {} channels, {} bits per sample.", channelCount, bitsPerSample);
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
	auto assetIterator = assets.emplace(
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
	auto audioClipInMap = assets.find(uuid);
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
