#include "OpenALSource.hpp"
#include "OpenALBuffer.hpp"

OpenALSource::OpenALSource() {
    alGenSources((ALuint)1, &source);
    
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
}

OpenALSource::OpenALSource(SoundSourceCreateInfo create_info) {
    alGenSources((ALuint)1, &source);
    
    alSourcef(source, AL_PITCH, 1);
}

OpenALSource::~OpenALSource() {
    alDeleteSources(1, &source);
}

void OpenALSource::Play(SoundBuffer *buffer) {
    Stop();
    SetBuffer(buffer);
    Play();
}

void OpenALSource::Play() {
    alSourcePlay(source);
}

void OpenALSource::Pause() {
    alSourcePause(source);
}

void OpenALSource::Stop() {
    alSourceStop(source);
}

void OpenALSource::SetBuffer(SoundBuffer *buffer) {
    OpenALBuffer *buff = (OpenALBuffer *)buffer;
    alSourcei(source, AL_BUFFER, buff->GetSoundBuffer());
}

void OpenALSource::SetPosition(float x, float y, float z) {
    alSource3f(source, AL_POSITION, x, y, z);
}

void OpenALSource::SetVelocity(float x, float y, float z) {
    alSource3f(source, AL_VELOCITY, x, y, z);
}

void OpenALSource::SetVolume(float volume) {
    alSourcef(source, AL_GAIN, volume);
}

void OpenALSource::SetPitch(float pitch) {
    alSourcef(source, AL_PITCH, pitch);
}

void OpenALSource::SetLooping(bool loops) {
    alSourcei(source, AL_LOOPING, loops);
}

bool OpenALSource::IsPlaying() {
    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);

    return (source_state == AL_PLAYING);
}
