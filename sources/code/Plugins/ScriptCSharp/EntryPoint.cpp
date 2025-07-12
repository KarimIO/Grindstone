#include "pch.hpp"
#include <chrono>
#include <string>
#include <EngineCore/Logger.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/Scenes/Scene.hpp"

#include "CSharpSystem.hpp"
#include "CSharpManager.hpp"
#include "Components/ScriptComponent.hpp"
#include "CSharpSystem.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone::Scripting::CSharp;

Plugins::Interface* globalPluginInterface = nullptr;

extern "C" {
	CSHARP_EXPORT void LoggerLog(int logSeverity, const char* message) {
		GPRINT(static_cast<LogSeverity>(logSeverity), LogSource::Scripting, message);
	}

	/*
	CSHARP_EXPORT void EntityCreateComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
		entt::registry& reg = scene->GetEntityRegistry();
		CSharpManager::GetInstance().CallCreateComponent(scene, entityHandle, monoType);
	}

	CSHARP_EXPORT void EntityHasComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
		entt::registry& reg = scene->GetEntityRegistry();
		CSharpManager::GetInstance().CallHasComponent(scene, entityHandle, monoType);
	}

	CSHARP_EXPORT void EntityRemoveComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
		entt::registry& reg = scene->GetEntityRegistry();
		CSharpManager::GetInstance().CallRemoveComponent(scene, entityHandle, monoType);
	}
	*/

	void QueueReloadCsharp() {
		CSharpManager::GetInstance().QueueReload();
	}

	CSHARP_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		CSharpManager& manager = CSharpManager::GetInstance();
		manager.Initialize();

		globalPluginInterface = pluginInterface;
		pluginInterface->RegisterComponent<ScriptComponent>(SetupCSharpScriptComponent, DestroyCSharpScriptComponent);
		pluginInterface->RegisterSystem("Scripting::CSharp::Update", UpdateSystem);
		pluginInterface->systemRegistrar->RegisterEditorSystem("Scripting::CSharp::UpdateEditor", UpdateEditorSystem);
		pluginInterface->SetReloadCsharpCallback(QueueReloadCsharp);
	}

	CSHARP_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->SetReloadCsharpCallback(nullptr);
		pluginInterface->systemRegistrar->UnregisterEditorSystem("Scripting::CSharp::UpdateEditor");
		pluginInterface->UnregisterSystem("Scripting::CSharp::Update");
		pluginInterface->UnregisterComponent<ScriptComponent>();

		CSharpManager& manager = CSharpManager::GetInstance();
		manager.Cleanup();
	}
}
