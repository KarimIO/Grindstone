#ifndef SOUND_SOURCE_HPP
#define SOUND_SOURCE_HPP

#include "SoundBuffer.hpp"

struct SoundSourceCreateInfo {
    float position[3];
    float velocity[3];
    float volume;
    float pitch;
    bool loops;
};

class SoundSource {
public:
    virtual ~SoundSource() = 0;

    virtual void Play(SoundBuffer *buffer) = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;

    virtual void SetBuffer(SoundBuffer *buffer) = 0;
    virtual void SetPosition(float x, float y, float z) = 0;
    virtual void SetVelocity(float x, float y, float z) = 0;
    virtual void SetVolume(float volume) = 0;
    virtual void SetPitch(float pitch) = 0;
    virtual void SetLooping(bool loops) = 0;

    virtual bool IsPlaying() = 0;
};

#endif