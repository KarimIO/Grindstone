#include <Grindstone.Audio.OpenAL/include/pch.hpp>
#include <entt/entt.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>

#include <Grindstone.Audio.OpenAL/include/Components/AudioListenerComponent.hpp>
#include <Grindstone.Audio.OpenAL/include/Components/AudioSourceComponent.hpp>
#include <Grindstone.Audio.OpenAL/include/AudioClipImporter.hpp>
#include <Grindstone.Audio.OpenAL/include/Core.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

Audio::Core* audioCore = nullptr;
Audio::AudioClipImporter* audioClipImporter = nullptr;

extern "C" {
	AUDIO_OPENAL_API void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		audioCore = AllocatorCore::Allocate<Audio::Core>();
		audioClipImporter = AllocatorCore::Allocate<Audio::AudioClipImporter>();

		pluginInterface->RegisterComponent<AudioListenerComponent>();
		pluginInterface->RegisterComponent<AudioSourceComponent>(SetupAudioSourceComponent, DestroyAudioSourceComponent);
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
