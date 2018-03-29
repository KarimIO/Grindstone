#ifndef AUDIO_OPENAL_HPP
#define AUDIO_OPENAL_HPP

#include "../AudioCommon/AudioCommon.hpp"

#include <AL/al.h>
#include <AL/alc.h>

class OpenALWrapper : public AudioWrapper {
public:
    OpenALWrapper();
    ~OpenALWrapper();

    virtual SoundBuffer* CreateBuffer(SoundBufferCreateInfo create_info);
    virtual SoundSource* CreateSource();
    virtual SoundSource* CreateSource(SoundSourceCreateInfo create_info);

	virtual void SetListenerPosition(float x, float y, float z);
	virtual void SetListenerVelocity(float x, float y, float z);
	virtual void SetListenerOrientation(float targ_x, float targ_y, float targ_z, float up_x, float up_y, float up_z);
private:
    ALCdevice *device;
    ALCcontext *context;
    
    void CheckForErrors();
    void EnumerateDevices(const ALCchar *devices);
};

#endif