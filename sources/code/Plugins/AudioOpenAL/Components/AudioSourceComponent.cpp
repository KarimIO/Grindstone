#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "../Core.hpp"
#include "AudioSourceComponent.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;

REFLECT_STRUCT_BEGIN(AudioSourceComponent)
	REFLECT_STRUCT_MEMBER(audioClip)
	REFLECT_STRUCT_MEMBER(isLooping)
	REFLECT_STRUCT_MEMBER(volume)
	REFLECT_STRUCT_MEMBER(pitch)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupAudioSourceComponent(entt::registry& registry, entt::entity entity) {
	Grindstone::Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	Grindstone::AudioSourceComponent& audioSource = registry.get<AudioSourceComponent>(entity);

	Grindstone::Audio::AudioClipAsset* audioClip = assetManager->GetAssetByUuid<Grindstone::Audio::AudioClipAsset>(audioSource.audioClip.uuid);
	if (audioClip == nullptr) {
		return;
	}

	Audio::Source::CreateInfo audioSourceCreateInfo{};
	audioSourceCreateInfo.audioClip = audioClip;
	audioSourceCreateInfo.isLooping = audioSource.isLooping;
	audioSourceCreateInfo.volume = audioSource.volume;
	audioSourceCreateInfo.pitch = audioSource.pitch;
	audioSource.source = AllocatorCore::Allocate<Audio::Source>(audioSourceCreateInfo);

	audioSource.source->Play();
}

void Grindstone::DestroyAudioSourceComponent(entt::registry& registry, entt::entity entity) {
	Grindstone::AudioSourceComponent& audioSource = registry.get<AudioSourceComponent>(entity);
	AllocatorCore::Free<Audio::Source>(audioSource.source);
}
