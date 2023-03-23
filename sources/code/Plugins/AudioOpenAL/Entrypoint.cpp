#include "pch.hpp"
#include "EngineCore/PluginSystem/Interface.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/EngineCore.hpp"

#include <entt/entt.hpp>
#include "Components/AudioListenerComponent.hpp"
#include "Components/AudioSourceComponent.hpp"
#include "AudioClipImporter.hpp"
#include "Core.hpp"
using namespace Grindstone;

extern "C" {
	AUDIO_OPENAL_API void InitializeModule(Plugins::Interface* pluginInterface) {
		EngineCore* engineCore = pluginInterface->GetEngineCore();
		Audio::Core::GetInstance().SetEngineCorePtr(engineCore);
		pluginInterface->RegisterComponent<AudioListenerComponent>();
		pluginInterface->RegisterComponent<AudioSourceComponent>(SetupAudioSourceComponent);
		pluginInterface->RegisterAssetType(AssetType::AudioClip, "AudioClip", new Audio::AudioClipImporter(engineCore));
	}

	AUDIO_OPENAL_API void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
