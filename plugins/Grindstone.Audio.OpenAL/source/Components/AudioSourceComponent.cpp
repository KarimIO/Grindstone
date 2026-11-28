#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Audio.OpenAL/include/AudioClip.hpp>
#include <Grindstone.Audio.OpenAL/include/Components/AudioSourceComponent.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

REFLECT_STRUCT_BEGIN(AudioSourceComponent)
	REFLECT_STRUCT_MEMBER(audioClip)
	REFLECT_STRUCT_MEMBER(isLooping)
	REFLECT_STRUCT_MEMBER(volume)
	REFLECT_STRUCT_MEMBER(pitch)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::AudioSourceComponent::Construct(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	Grindstone::Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;
	Grindstone::AudioSourceComponent& audioSource = cxtSet.GetEntityRegistry().get<AudioSourceComponent>(entity);

	Grindstone::Audio::AudioClipAsset* audioClip = audioSource.audioClip.Get();
	if (audioClip == nullptr) {
		return;
	}

	Audio::Source::CreateInfo audioSourceCreateInfo{};
	audioSourceCreateInfo.audioClip = audioClip;
	audioSourceCreateInfo.isLooping = audioSource.isLooping;
	audioSourceCreateInfo.volume = audioSource.volume;
	audioSourceCreateInfo.pitch = audioSource.pitch;
	audioSource.source = AllocatorCore::Allocate<Audio::Source>(audioSourceCreateInfo);

	if (audioSource.playOnStart) {
		audioSource.source->Play();
	}
}

void Grindstone::AudioSourceComponent::Play() {
	if (source != nullptr) {
		source->Play();
	}
}

void Grindstone::AudioSourceComponent::Pause() {
	if (source != nullptr) {
		source->Pause();
	}
}

void Grindstone::AudioSourceComponent::Stop() {
	if (source != nullptr) {
		source->Stop();
	}
}

void Grindstone::AudioSourceComponent::Destroy(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	Grindstone::AudioSourceComponent& audioSource = cxtSet.GetEntityRegistry().get<AudioSourceComponent>(entity);
	AllocatorCore::Free<Audio::Source>(audioSource.source);
}

extern "C" {
	AUDIO_OPENAL_API void AudioSourceComponentPlay(Grindstone::AudioSourceComponent& comp) {
		comp.Play();
	}

	AUDIO_OPENAL_API void AudioSourceComponentPause(Grindstone::AudioSourceComponent& comp) {
		comp.Pause();
	}

	AUDIO_OPENAL_API void AudioSourceComponentStop(Grindstone::AudioSourceComponent& comp) {
		comp.Stop();
	}
} 
