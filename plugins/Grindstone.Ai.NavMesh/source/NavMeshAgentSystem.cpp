#include <Grindstone.Ai.NavMesh/include/NavMeshAgentSystem.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavAgentComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshWorldContext.hpp>

#include <EngineCore/EngineCore.hpp>

#include <DetourNavMeshQuery.h>

void Grindstone::Ai::NavMeshAgentSystem(Grindstone::WorldContextSet& worldContextSet) {
	Grindstone::Ai::NavMeshWorldContext* cxt = static_cast<Grindstone::Ai::NavMeshWorldContext*>(worldContextSet.GetContext(Grindstone::Ai::navMeshWorldContextName));
	if (cxt == nullptr) {
		return;
	}

	const float deltaTime = EngineCore::GetInstance().GetDeltaTime();

	entt::registry& registry = worldContextSet.GetEntityRegistry();
	auto view = registry.view<Grindstone::Ai::NavAgentComponent, Grindstone::TransformComponent>();
	view.each(
		[&cxt](
			Grindstone::Ai::NavAgentComponent& navAgentComponent,
			const Grindstone::TransformComponent& transformComponent
		) {
			const dtNavMeshQuery* navMeshQuery = navAgentComponent.GetNavMeshQuery();
		}
	);
}
