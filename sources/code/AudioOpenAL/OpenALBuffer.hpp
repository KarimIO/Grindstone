#ifndef OPENAL_BUFFER_HPP
#define OPENAL_BUFFER_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include "../AudioCommon/SoundBuffer.hpp"

class OpenALBuffer : public SoundBuffer {
public:
    OpenALBuffer(SoundBufferCreateInfo create_info);
    virtual ~OpenALBuffer();

    ALuint GetSoundBuffer();

private:
    ALuint buffer;
};

#endif