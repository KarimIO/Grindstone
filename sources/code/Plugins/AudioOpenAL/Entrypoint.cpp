#include "pch.hpp"
#include "EngineCore/PluginSystem/Interface.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/EngineCore.hpp"

#include <entt/entt.hpp>
#include "Components/AudioListenerComponent.hpp"
#include "Components/AudioSourceComponent.hpp"
using namespace Grindstone;

extern "C" {
	AUDIO_OPENAL_API void initializeModule(Plugins::Interface* pluginInterface) {
		EngineCore* engineCore = pluginInterface->getEngineCore();
		auto componentRegistrar = engineCore->GetComponentRegistrar();
		componentRegistrar->RegisterComponent<AudioListenerComponent>();
		componentRegistrar->RegisterComponent<AudioSourceComponent>();
	}

	AUDIO_OPENAL_API void releaseModule(Plugins::Interface* pluginInterface) {
	}
}
