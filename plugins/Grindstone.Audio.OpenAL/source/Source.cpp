#include <Grindstone.Audio.OpenAL/include/Source.hpp>
using namespace Grindstone::Audio;

Source::Source() {
	alGenSources(1, &source);
}

Source::Source(CreateInfo& createInfo) {
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, createInfo.pitch);
	alSourcef(source, AL_GAIN, createInfo.volume);
	alSource3f(source, AL_POSITION, createInfo.position[0], createInfo.position[1], createInfo.position[2]);
	alSource3f(source, AL_VELOCITY, createInfo.velocity[0], createInfo.velocity[1], createInfo.velocity[2]);
	alSourcei(source, AL_LOOPING, createInfo.isLooping ? AL_TRUE : AL_FALSE);
	alSourcei(source, AL_BUFFER, createInfo.audioClip->buffer);
}

Source::~Source() {
	alDeleteSources(1, &source);
}

void Source::Play() {
	alSourcePlay(source);
}

void Source::Pause() {
	alSourcePause(source);
}

void Source::SetPitch(float pitch) {
	alSourcef(source, AL_PITCH, pitch);
}

void Source::SetVolume(float volume) {
	alSourcef(source, AL_GAIN, volume);
}

void Source::SetPosition(float x, float y, float z) {
	alSource3f(source, AL_POSITION, x, y, z);
}

void Source::SetVelocity(float x, float y, float z) {
	alSource3f(source, AL_VELOCITY, x, y, z);
}

void Source::SetBuffer(Audio::AudioClipAsset* audioClip) {
	alSourcei(source, AL_BUFFER, audioClip->buffer);
}

void Source::SetIsLooping(bool isLooping) {
	alSourcei(source, AL_LOOPING, isLooping ? AL_TRUE : AL_FALSE);
}

bool Source::IsPlaying() {
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);

	return state == AL_PLAYING;
}
