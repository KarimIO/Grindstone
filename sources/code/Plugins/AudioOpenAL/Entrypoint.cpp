#include "pch.hpp"
#include "EngineCore/PluginSystem/Interface.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/EngineCore.hpp"

#include <entt/entt.hpp>
#include "Components/AudioListenerComponent.hpp"
#include "Components/AudioSourceComponent.hpp"
#include "Core.hpp"
using namespace Grindstone;

void SetupAudioSourceComponent(entt::registry& registry, entt::entity entity, void* componentPtr) {
	Audio::Core& core = Audio::Core::GetInstance();

	auto audioSource = (AudioSourceComponent*)componentPtr;

	std::string path = std::string("../compiledAssets/") + audioSource->audioClip;
	Audio::Clip* clip = core.CreateClip(path.c_str());
	
	Audio::Source::CreateInfo audioSourceCreateInfo{};
	audioSourceCreateInfo.audioClip = clip;
	audioSourceCreateInfo.isLooping = audioSource->isLooping;
	audioSourceCreateInfo.volume = audioSource->volume;
	audioSourceCreateInfo.pitch = audioSource->pitch;
	audioSource->source = core.CreateSource(audioSourceCreateInfo);

	audioSource->source->Play();
}

extern "C" {
	AUDIO_OPENAL_API void initializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->RegisterComponent<AudioListenerComponent>();
		pluginInterface->RegisterComponent<AudioSourceComponent>(SetupAudioSourceComponent);
	}

	AUDIO_OPENAL_API void releaseModule(Plugins::Interface* pluginInterface) {
	}
}
