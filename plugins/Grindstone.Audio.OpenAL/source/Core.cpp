#include <iostream>

#include <EngineCore/Logger.hpp>

#include <Grindstone.Audio.OpenAL/include/Core.hpp>

using namespace Grindstone;
using namespace Grindstone::Audio;

Grindstone::Audio::Core* Grindstone::Audio::Core::instance = nullptr;

static bool CheckOpenALErrors(const std::string& filename, const std::uint_fast32_t line) {
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		GPRINT_ERROR_V(LogSource::Audio, "***ERROR*** ({0}: {1})", filename, line);
		switch (error) {
		case AL_INVALID_NAME:
			GPRINT_ERROR(LogSource::Audio, "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function");
			break;
		case AL_INVALID_ENUM:
			GPRINT_ERROR(LogSource::Audio, "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function");
			break;
		case AL_INVALID_VALUE:
			GPRINT_ERROR(LogSource::Audio, "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function");
			break;
		case AL_INVALID_OPERATION:
			GPRINT_ERROR(LogSource::Audio, "AL_INVALID_OPERATION: the requested operation is not valid");
			break;
		case AL_OUT_OF_MEMORY:
			GPRINT_ERROR(LogSource::Audio, "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
			break;
		default:
			GPRINT_ERROR_V(LogSource::Audio, "UNKNOWN AL ERROR: {0}", error);
		}
		return false;
	}
	return true;
}

Core::Core() {
	device = alcOpenDevice(nullptr);
	if (!device) {
		GPRINT_FATAL(LogSource::Audio, "Could not create OpenAL Device.");
	}

	context = alcCreateContext(device, nullptr);
	if (!context) {
		GPRINT_FATAL(LogSource::Audio, "Could not create OpenAL Context.");
	}

	if (!alcMakeContextCurrent(context)) {
		GPRINT_FATAL(LogSource::Audio, "Could not create OpenAL Context Current.");
	}

	instance = this;
}

Core::~Core() {
	if (context) {
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
	}

	if (device) {
		if (alcCloseDevice(device)) {
			GPRINT_ERROR(LogSource::Audio, "Could not close OpenAL Device.");
		}
	}
}

Core& Core::GetInstance() {
	return *instance;
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
