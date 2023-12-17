#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "AudioSourceComponent.hpp"
#include "../Core.hpp"

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(AudioSourceComponent)
	REFLECT_STRUCT_MEMBER(audioClip)
	REFLECT_STRUCT_MEMBER(isLooping)
	REFLECT_STRUCT_MEMBER(volume)
	REFLECT_STRUCT_MEMBER(pitch)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::SetupAudioSourceComponent(ECS::Entity& entity, void* componentPtr) {
	Audio::Core& core = Audio::Core::GetInstance();
	Grindstone::Assets::AssetManager* assetManager = core.engineCore->assetManager;

	Grindstone::AudioSourceComponent* audioSource = static_cast<AudioSourceComponent*>(componentPtr);

	Grindstone::Audio::AudioClipAsset* audioClip = assetManager->GetAsset<Grindstone::Audio::AudioClipAsset>(audioSource->audioClip.uuid);
	if (audioClip == nullptr) {
		return;
	}

	Audio::Source::CreateInfo audioSourceCreateInfo{};
	audioSourceCreateInfo.audioClip = audioClip;
	audioSourceCreateInfo.isLooping = audioSource->isLooping;
	audioSourceCreateInfo.volume = audioSource->volume;
	audioSourceCreateInfo.pitch = audioSource->pitch;
	audioSource->source = core.CreateSource(audioSourceCreateInfo);

	audioSource->source->Play();
}
