#include <EngineCore/PluginSystem/Interface.hpp>

#include <Grindstone.Ai.NavMesh/include/pch.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshSystem.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshWorldContext.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavMeshComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavAgentComponent.hpp>

#include <Recast.h>

using namespace Grindstone;

extern "C" {
	AI_NAVMESH_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());

	}

	AI_NAVMESH_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
