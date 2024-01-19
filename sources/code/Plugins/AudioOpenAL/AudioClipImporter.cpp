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

using namespace Grindstone::Audio;

AudioClipImporter::AudioClipImporter(EngineCore* engineCore) {
	this->engineCore = engineCore;
}

void* AudioClipImporter::ProcessLoadedFile(Uuid uuid) {
	/*
	char* fileContent;
	size_t fileSize;
	if (!engineCore->assetManager->LoadFile(uuid, fileContent, fileSize)) {
		std::string errorMsg = "AudioClipImporter::ProcessLoadedFile Unable to load file with id " + uuid.ToString() + ".";
		engineCore->Print(LogSeverity::Error, errorMsg.c_str());
		return nullptr;
	}
	*/

	std::filesystem::path path = engineCore->GetAssetPath(uuid.ToString());
	std::string pathString = path.string();
	const char* pathCstr = pathString.c_str();
	if (!std::filesystem::exists(path)) {
		std::string errorMsg = std::string("AudioClipImporter::LoadFromPath - Could not find file:") + pathString;
		engineCore->Print(LogSeverity::Error, errorMsg.c_str());
		return nullptr;
	}

	drwav wav;
	if (!drwav_init_file(&wav, pathCstr, nullptr)) {
		std::string errorMsg = std::string("AudioClipImporter::LoadFromPath - Failed to load file:") + pathString;
		engineCore->Print(LogSeverity::Error, errorMsg.c_str());
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
		throw std::runtime_error(
			std::string("ERROR: unrecognised wave format: ") +
			std::to_string(channelCount) + " channels, " +
			std::to_string(bitsPerSample) + " bits per sample."
		);
	}

	alBufferData(buffer, format, memoryBuffer, static_cast<ALsizei>(fileSize), sampleRate);

	auto& asset = assets.emplace(
		uuid,
		AudioClipAsset(uuid, uuid.ToString(), buffer, channelCount, sampleRate, bitsPerSample)
	);

	drwav_free(memoryBuffer, nullptr);
	drwav_uninit(&wav);

	return &asset.first->second;
}

void AudioClipImporter::QueueReloadAsset(Uuid uuid) {
	auto audioClipInMap = assets.find(uuid);
	if (audioClipInMap == assets.end()) {
		return;
	}

	std::filesystem::path path = engineCore->GetAssetPath(uuid.ToString());
	std::string pathString = path.string();
	const char* pathCstr = pathString.c_str();
	if (!std::filesystem::exists(path)) {
		std::string errorMsg = std::string("AudioClipImporter::LoadFromPath - Could not find file:") + pathString;
		engineCore->Print(LogSeverity::Error, errorMsg.c_str());
		return;
	}

	drwav wav;
	if (!drwav_init_file(&wav, pathCstr, nullptr)) {
		std::string errorMsg = std::string("AudioClipImporter::LoadFromPath - Failed to load file:") + pathString;
		engineCore->Print(LogSeverity::Error, errorMsg.c_str());
		return;
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
		throw std::runtime_error(
			std::string("ERROR: unrecognised wave format: ") +
			std::to_string(channelCount) + " channels, " +
			std::to_string(bitsPerSample) + " bits per sample."
		);
	}

	alDeleteBuffers(1, &audioClipInMap->second.buffer);

	alBufferData(buffer, format, memoryBuffer, static_cast<ALsizei>(fileSize), sampleRate);

	audioClipInMap->second = AudioClipAsset(uuid, uuid.ToString(), buffer, channelCount, sampleRate, bitsPerSample);

	drwav_free(memoryBuffer, nullptr);
	drwav_uninit(&wav);
}
