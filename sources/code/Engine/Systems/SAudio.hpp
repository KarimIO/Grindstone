#ifndef _S_AUDIO_H
#define _S_AUDIO_H

#include <AudioCommon.hpp>
#include <SoundBuffer.hpp>
#include <SoundSource.hpp>
#include "CBase.hpp"
#include <vector>

class CAudioSource : public CBase {
public:
    CAudioSource(unsigned int entID, SoundSource *source);
    inline void Play(SoundBuffer *buffer);
    inline void Play();
    inline void Pause();
    inline void Stop();

    inline void SetBuffer(SoundBuffer *buffer);
    inline void SetPosition(float x, float y, float z);
    inline void SetVelocity(float x, float y, float z);
    inline void SetVolume(float volume);
    inline void SetPitch(float pitch);
    inline void SetLooping(bool loops);

    inline bool IsPlaying();
private:
    SoundSource *source_;
};

class CAudioListener : public CBase {
public:
    CAudioListener();
    bool disabled_;
};

class SAudio {
public:
    void Initialize(AudioWrapper *audio_wrapper);
    void AddSource(unsigned int entID, unsigned int &target);
    void AddBuffer(unsigned int id, std::string path);
    void AddListener(unsigned int entID, unsigned int & target);
    void Update();
    void UpdateListenerData();
    void UpdateSourceData();
    CAudioSource *GetComponent(unsigned int id);
private:
    AudioWrapper *audio_wrapper_;
    CAudioListener listener_;
    std::vector<SoundBuffer> buffers_;
    std::vector<CAudioSource> sources_;
};

#endif