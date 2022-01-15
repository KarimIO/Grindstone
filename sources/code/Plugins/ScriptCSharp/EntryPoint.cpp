#include "pch.hpp"
#include <chrono>
#include <string>
#include <EngineCore/PluginSystem/Interface.hpp>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"

#include "CSharpSystem.hpp"
#include "CSharpManager.hpp"
#include "Components/ScriptComponent.hpp"
#include "CSharpSystem.hpp"
using namespace Grindstone::Scripting::CSharp;

extern "C" {
	CSHARP_EXPORT void initializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->componentRegistrar->RegisterComponent<ScriptComponent>();

		pluginInterface->systemRegistrar->RegisterSystem("Scripting::CSharp::Update", UpdateSystem);
	}

	CSHARP_EXPORT void releaseModule(Plugins::Interface* pluginInterface) {
	}
}
