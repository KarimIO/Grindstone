#if 0
//#ifndef _S_AUDIO_H
#define _S_AUDIO_H

#include <AudioCommon.hpp>
#include <SoundBuffer.hpp>
#include <SoundSource.hpp>
#include "CBase.hpp"
#include <vector>

class CAudioSource : public CBase {
public:
    CAudioSource(unsigned int entID, SoundSource *source);
    void Play(SoundBuffer *buffer);
    void Play();
    void Pause();
    void Stop();

    void SetBuffer(SoundBuffer *buffer);
    void SetPosition(float x, float y, float z);
    void SetVelocity(float x, float y, float z);
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLooping(bool loops);

    bool IsPlaying();
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
    void AddAutoplaySource(unsigned int id);
    void PlayAutoplay();
    void Update();
    void UpdateListenerData();
    void UpdateSourceData();
    CAudioSource *GetComponent(unsigned int id);
private:
    AudioWrapper *audio_wrapper_;
    CAudioListener listener_;
    std::vector<SoundBuffer> buffers_;
    std::vector<CAudioSource> sources_;
    std::vector<unsigned int> autoplay_sources_;
};

#endif