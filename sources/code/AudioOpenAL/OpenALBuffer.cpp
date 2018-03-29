#include "OpenALBuffer.hpp"
#include <stdexcept>

OpenALBuffer::OpenALBuffer(SoundBufferCreateInfo create_info) {
    alGenBuffers((ALuint)1, &buffer);

    bool stereo = (create_info.channels > 1);
    ALenum format;
    switch (create_info.samples) {
    case 16:
        if (stereo)
            format = AL_FORMAT_STEREO16;
        else
            format = AL_FORMAT_MONO16;
        break;
    case 8:
        if (stereo)
            format = AL_FORMAT_STEREO8;
        else
            format = AL_FORMAT_MONO8;
        break;
    default:
        throw std::runtime_error("Invalid sound buffer format.");
    }

    alBufferData(buffer, format, create_info.data, create_info.size, create_info.frequency);
}

OpenALBuffer::~OpenALBuffer() {
    alDeleteBuffers(1, &buffer);
}

ALuint OpenALBuffer::GetSoundBuffer() {
    return buffer;
}