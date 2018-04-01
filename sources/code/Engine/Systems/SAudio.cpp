#include "SAudio.hpp"
#include "../Core/Engine.hpp"

#define DR_WAV_IMPLEMENTATION
#include "../../../deps/dr_wav.h"

CAudioSource::CAudioSource(unsigned int entID, SoundSource *source) : source_(source) {
    entityID = entID;
    componentType = COMPONENT_AUDIO_SOURCE;
}

void CAudioSource::Play(SoundBuffer *buffer) {
    source_->Play(buffer);
}

void CAudioSource::Play() {
    source_->Play();
}

void CAudioSource::Pause() {
    source_->Pause();
}

void CAudioSource::Stop() {
    source_->Stop();
}

void CAudioSource::SetBuffer(SoundBuffer *buffer) {
    source_->SetBuffer(buffer);
}

void CAudioSource::SetPosition(float x, float y, float z) {
    source_->SetPosition(x, y, z);
}

void CAudioSource::SetVelocity(float x, float y, float z) {
    source_->SetVelocity(x, y, z);
}

void CAudioSource::SetVolume(float volume) {
    source_->SetVolume(volume);
}

void CAudioSource::SetPitch(float pitch) {
    source_->SetPitch(pitch);
}

void CAudioSource::SetLooping(bool loops) {
    source_->SetLooping(loops);
}

bool CAudioSource::IsPlaying() {
    source_->IsPlaying();
}

CAudioListener::CAudioListener() : disabled_(true) {}

void SAudio::Initialize(AudioWrapper *audio_wrapper) {
    audio_wrapper_ = audio_wrapper;
}

void SAudio::AddBuffer(unsigned int id, std::string path) {
    drwav *wav = drwav_open_file(path.c_str());
    if (wav == NULL) {
		throw std::runtime_error("Error opening and reading WAV file.");
    }

	SoundBufferCreateInfo create_info;
    create_info.channels = wav->channels;
    create_info.samples = wav->bitsPerSample;
    create_info.frequency = wav->sampleRate;

    create_info.size = wav->totalSampleCount * wav->bytesPerSample;
    create_info.data = (char *)malloc(create_info.size);
    drwav_read_raw(wav, create_info.size, create_info.data);


	SoundBuffer *sound_buffer_ = audio_wrapper_->CreateBuffer(create_info);
	sources_[id].SetBuffer(sound_buffer_);
    
	drwav_free(create_info.data);
    drwav_close(wav);
}

void SAudio::AddSource(unsigned int entID, unsigned int &target) {
    sources_.emplace_back(entID, audio_wrapper_->CreateSource());
}

void SAudio::AddListener(unsigned int entID, unsigned int & target) {
    listener_.entityID = entID;
    target = 1;
    listener_.disabled_ = false;
}

void SAudio::AddAutoplaySource(unsigned int id) {
    autoplay_sources_.push_back(id);
}

void SAudio::PlayAutoplay() {
    for (unsigned int i : autoplay_sources_) {
        sources_[i].Play();
    }
}

void SAudio::Update() {
    UpdateListenerData();
    UpdateSourceData();
}

CAudioSource *SAudio::GetComponent(unsigned int id) {
    return &sources_[id];
}

void SAudio::UpdateSourceData() {
    for (auto &source : sources_) {
        Entity *ent = &engine.entities[source.entityID];
        CTransform *transform = &engine.transformSystem.components[ent->components_[COMPONENT_TRANSFORM]];
        glm::vec3 pos = transform->position;
        glm::vec3 vel = transform->velocity;
        
        source.SetPosition(pos.x, pos.y, pos.z);
        source.SetVelocity(vel.x, vel.y, vel.z);
    }
}

void SAudio::UpdateListenerData() {
    if (!listener_.disabled_) {
        Entity *ent = &engine.entities[listener_.entityID];
        CTransform *transform = &engine.transformSystem.components[ent->components_[COMPONENT_TRANSFORM]];
        glm::vec3 pos = transform->position;
        glm::vec3 vel = transform->velocity;
        glm::vec3 fwd = transform->GetForward();
        glm::vec3 up = transform->GetUp();

        audio_wrapper_->SetListenerPosition(pos.x, pos.y, pos.z);
        audio_wrapper_->SetListenerVelocity(vel.x, vel.y, vel.z);
        audio_wrapper_->SetListenerOrientation(fwd.x, fwd.y, fwd.z, up.x, up.y, up.z);
    }
}