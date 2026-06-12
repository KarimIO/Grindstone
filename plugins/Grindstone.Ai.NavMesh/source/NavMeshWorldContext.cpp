#include <Grindstone.Ai.NavMesh/include/NavMeshWorldContext.hpp>

static Grindstone::Ai::NavMeshWorldContext* navMeshActiveContext = nullptr;

Grindstone::Ai::NavMeshWorldContext* Grindstone::Ai::NavMeshWorldContext::GetActiveContext() {
	return navMeshActiveContext;
}

void Grindstone::Ai::NavMeshWorldContext::SetActiveContext(NavMeshWorldContext& cxt) {
	navMeshActiveContext = &cxt;
}

void Grindstone::Ai::NavMeshWorldContext::SetAsActive() {
	navMeshActiveContext = this;
}

dtNavMesh* Grindstone::Ai::NavMeshWorldContext::GetNavMesh() {
    return navMesh;
}
