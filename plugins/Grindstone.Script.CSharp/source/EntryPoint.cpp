#include <Grindstone.Script.CSharp/include/pch.hpp>
#include <chrono>
#include <string>
#include <EngineCore/Logger.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>

#include <Grindstone.Script.CSharp/include/CSharpSystem.hpp>
#include <Grindstone.Script.CSharp/include/CSharpManager.hpp>
#include <Grindstone.Script.CSharp/include/Components/ScriptComponent.hpp>
#include <Grindstone.Script.CSharp/include/CSharpSystem.hpp>

using namespace Grindstone::Memory;
using namespace Grindstone::Scripting::CSharp;

Plugins::Interface* globalPluginInterface = nullptr;

extern "C" {
	CSHARP_EXPORT void LoggerLog(int logSeverity, const char* message) {
		GPRINT(static_cast<LogSeverity>(logSeverity), LogSource::Scripting, message);
	}

	void QueueReloadCsharp() {
		CSharpManager::GetInstance().QueueReload();
	}

	CSHARP_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		CSharpManager& manager = CSharpManager::GetInstance();
		manager.Initialize();

		globalPluginInterface = pluginInterface;
		Grindstone::EngineCore::GetInstance().RegisterService<CSharpManager>(&manager);
		pluginInterface->RegisterComponent<ScriptComponent>();
		pluginInterface->RegisterSystem("Scripting::CSharp::Update", UpdateSystem);
		pluginInterface->RegisterEditorSystem("Scripting::CSharp::UpdateEditor", UpdateEditorSystem);

	}

	CSHARP_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterEditorSystem("Scripting::CSharp::UpdateEditor");
		pluginInterface->UnregisterSystem("Scripting::CSharp::Update");
		pluginInterface->UnregisterComponent<ScriptComponent>();
		Grindstone::EngineCore::GetInstance().UnregisterService<CSharpManager>();

		CSharpManager& manager = CSharpManager::GetInstance();
		manager.Cleanup();
	}
}
