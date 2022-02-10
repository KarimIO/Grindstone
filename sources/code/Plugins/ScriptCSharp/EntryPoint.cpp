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

Plugins::Interface* globalPluginInterface = nullptr;

extern "C" {
	CSHARP_EXPORT void LoggerLog(int logSeverity, const char* message) {
		globalPluginInterface->Print((LogSeverity)logSeverity, message);
	}

	CSHARP_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		auto& manager = CSharpManager::GetInstance();
		manager.Initialize(pluginInterface->GetEngineCore());
		globalPluginInterface = pluginInterface;
		pluginInterface->RegisterComponent<ScriptComponent>(SetupCSharpScriptComponent);
		pluginInterface->RegisterSystem("Scripting::CSharp::Update", UpdateSystem);
		pluginInterface->systemRegistrar->RegisterEditorSystem("Scripting::CSharp::UpdateEditor", UpdateEditorSystem);
	}

	CSHARP_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
