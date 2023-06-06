#include "pch.hpp"
#include <chrono>
#include <string>
#include <EngineCore/PluginSystem/Interface.hpp>
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/Scenes/Scene.hpp"

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

	void ReloadCsharp() {
		CSharpManager::GetInstance().Reload();
	}

	CSHARP_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		auto& manager = CSharpManager::GetInstance();
		manager.Initialize(pluginInterface->GetEngineCore());
		globalPluginInterface = pluginInterface;
		pluginInterface->RegisterComponent<ScriptComponent>(SetupCSharpScriptComponent);
		pluginInterface->RegisterSystem("Scripting::CSharp::Update", UpdateSystem);
		pluginInterface->systemRegistrar->RegisterEditorSystem("Scripting::CSharp::UpdateEditor", UpdateEditorSystem);
		pluginInterface->SetReloadCsharpCallback(ReloadCsharp);

		manager.RegisterComponents();
	}

	CSHARP_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
