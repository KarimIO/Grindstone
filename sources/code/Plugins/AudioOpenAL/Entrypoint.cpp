#include "pch.hpp"
#include "EngineCore/PluginSystem/Interface.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include "EngineCore/EngineCore.hpp"

#include <entt/entt.hpp>
#include "Components/AudioListenerComponent.hpp"
#include "Components/AudioSourceComponent.hpp"
#include "AudioClipImporter.hpp"
#include "Core.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;

Audio::Core* audioCore = nullptr;
Audio::AudioClipImporter* audioClipImporter = nullptr;

extern "C" {
	AUDIO_OPENAL_API void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		audioCore = AllocatorCore::Allocate<Audio::Core>();

		EngineCore* engineCore = pluginInterface->GetEngineCore();
		Audio::Core::GetInstance().SetEngineCorePtr(engineCore);

		audioClipImporter = AllocatorCore::Allocate<Audio::AudioClipImporter>(engineCore);
		pluginInterface->RegisterComponent<AudioListenerComponent>();
		pluginInterface->RegisterComponent<AudioSourceComponent>(SetupAudioSourceComponent);
		pluginInterface->RegisterAssetType(AssetType::AudioClip, "AudioClip", audioClipImporter);
	}

	AUDIO_OPENAL_API void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterAssetType(AssetType::AudioClip);
		pluginInterface->UnregisterComponent<AudioSourceComponent>();
		pluginInterface->UnregisterComponent<AudioListenerComponent>();
		AllocatorCore::Free(audioClipImporter);
		AllocatorCore::Free(audioCore);
	}
}
