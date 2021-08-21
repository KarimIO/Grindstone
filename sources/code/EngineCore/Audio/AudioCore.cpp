#include <iostream>
#include "AudioCore.hpp"
using namespace Grindstone::Audio;

bool CheckOpenALErrors(const std::string& filename, const std::uint_fast32_t line) {
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
		switch (error) {
		case AL_INVALID_NAME:
			std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
			break;
		case AL_INVALID_ENUM:
			std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
			break;
		case AL_INVALID_VALUE:
			std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
			break;
		case AL_INVALID_OPERATION:
			std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
			break;
		case AL_OUT_OF_MEMORY:
			std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
			break;
		default:
			std::cerr << "UNKNOWN AL ERROR: " << error;
		}
		std::cerr << std::endl;
		return false;
	}
	return true;
}

Core::Core() {
	device = alcOpenDevice(nullptr);
	if (!device) {
		throw std::runtime_error("Could not create OpenAL Device.");
	}

	context = alcCreateContext(device, nullptr);
	if (!context) {
		throw std::runtime_error("Could not create OpenAL Context.");
	}

	if (!alcMakeContextCurrent(context)) {
		throw std::runtime_error("Could not create OpenAL Context Current.");
	}
}

Core::~Core() {
	if (context) {
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
	}

	if (device) {
		if (alcCloseDevice(device)) {
			std::cout << "Could not close OpenAL Device.\n";
		}
	}
}

bool Core::GetAvailableDevices(std::vector<std::string>& devicesVec, ALCdevice* device) {
	const ALCchar* devices = alcGetString(device, ALC_DEVICE_SPECIFIER);
	if (devices == nullptr) {
		throw std::runtime_error("Could not get OpenAL Devices.");
	}

	const char* ptr = devices;

	devicesVec.clear();
	do {
		devicesVec.push_back(std::string(ptr));
		ptr += devicesVec.back().size() + 1;
	} while (*(ptr + 1) != '\0');

	return true;
}

Clip* Core::CreateClip(const char* path) {
	return new Audio::Clip(path);
}

Source* Core::CreateSource(Audio::Source::CreateInfo& createInfo) {
	return new Audio::Source(createInfo);
}
