#ifndef SOUND_BUFFER_HPP
#define SOUND_BUFFER_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include "../AudioCommon/SoundBuffer.hpp"

struct SoundBufferCreateInfo {
    char *data;
    unsigned int size;
    unsigned short channels;
    unsigned short samples;
    unsigned int frequency;
};

class SoundBuffer {
public:
    virtual ~SoundBuffer() = 0;
};

#endif