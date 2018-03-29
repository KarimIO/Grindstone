#include "OpenALWrapper.hpp"
#include "OpenALSource.hpp"
#include "OpenALBuffer.hpp"
#include <stdexcept>
#include <cstring>

void OpenALWrapper::EnumerateDevices(const ALCchar *devices)
{
    const ALCchar *device = devices, *next = devices + 1;
    size_t len = 0;

    fprintf(stdout, "Devices list:\n");
    fprintf(stdout, "----------\n");
    while (device && *device != '\0' && next && *next != '\0') {
        fprintf(stdout, "%s\n", device);
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
    fprintf(stdout, "----------\n");
}

OpenALWrapper::OpenALWrapper() {
    // Create Audio Device
    device = alcOpenDevice(NULL);

    // Check for Errors
    if (!device)
        throw std::runtime_error("Cannot initialize OpenAL");

    ALboolean enumeration;

    enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_FALSE)
        throw std::runtime_error("Cannot get audio devices");

    EnumerateDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context))
        throw std::runtime_error("Cannot create audio context");

    SetListenerPosition(0.f, 0.f, 0.f);
    SetListenerVelocity(0.f, 0.f, 0.f);
    SetListenerOrientation(0.f, 0.f, 1.f, 0.f, 1.f, 0.f);
}

OpenALWrapper::~OpenALWrapper() {
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

SoundBuffer* OpenALWrapper::CreateBuffer(SoundBufferCreateInfo create_info) {
    return (SoundBuffer*)(new OpenALBuffer(create_info));
}

SoundSource* OpenALWrapper::CreateSource() {
    return (SoundSource*)(new OpenALSource());
}

SoundSource* OpenALWrapper::CreateSource(SoundSourceCreateInfo create_info) {
    return (SoundSource*)(new OpenALSource(create_info));
}

void OpenALWrapper::CheckForErrors() {
    ALCenum error = alGetError();
    if (error != AL_NO_ERROR)
        throw std::runtime_error("OpenAL Error!");
}

void OpenALWrapper::SetListenerPosition(float x, float y, float z) {
    alListener3f(AL_POSITION, x, y, z);
}

void OpenALWrapper::SetListenerVelocity(float x, float y, float z) {
    alListener3f(AL_VELOCITY, x, y, z);
}

void OpenALWrapper::SetListenerOrientation(float targ_x, float targ_y, float targ_z, float up_x, float up_y, float up_z) {
    ALfloat orinetation[] = { targ_x, targ_y, targ_z, up_x, up_y, up_z };
    alListenerfv(AL_ORIENTATION, orinetation);
}


AUDIO_EXPORT AudioWrapper* createAudio() {
	return new OpenALWrapper();
}

AUDIO_EXPORT void deleteAudio(AudioWrapper * ptr) {
    OpenALWrapper *wrapper = (OpenALWrapper *)ptr;
	delete wrapper;
}