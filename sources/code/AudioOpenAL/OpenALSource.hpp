#ifndef OPENAL_SOURCE_HPP
#define OPENAL_SOURCE_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include "../AudioCommon/SoundSource.hpp"

class OpenALSource {
public:
    OpenALSource();
    OpenALSource(SoundSourceCreateInfo create_info);
    virtual ~OpenALSource();

    virtual void Play(SoundBuffer *buffer);
    virtual void Play();
    virtual void Pause();
    virtual void Stop();

    virtual void SetBuffer(SoundBuffer *buffer);
    virtual void SetPosition(float x, float y, float z);
    virtual void SetVelocity(float x, float y, float z);
    virtual void SetVolume(float volume);
    virtual void SetPitch(float pitch);
    virtual void SetLooping(bool loops);
    
    virtual bool IsPlaying();
private:
    ALuint source;
};

#endif