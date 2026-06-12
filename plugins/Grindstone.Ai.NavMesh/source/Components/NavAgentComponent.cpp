#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/Scenes/Scene.hpp>

#include <Grindstone.Ai.NavMesh/include/pch.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavAgentComponent.hpp>

#include <DetourNavMeshQuery.h>
#include <EngineCore/Logger.hpp>

using namespace Grindstone::Ai;

REFLECT_STRUCT_BEGIN(NavAgentComponent)
	REFLECT_STRUCT_MEMBER(typeId)
	REFLECT_STRUCT_MEMBER(baseOffset)
	REFLECT_STRUCT_MEMBER(movementSpeed)
	REFLECT_STRUCT_MEMBER(angularSpeed)
	REFLECT_STRUCT_MEMBER(acceleration)
	REFLECT_STRUCT_MEMBER(stoppingDistance)
	REFLECT_STRUCT_MEMBER(autoBraking)
	REFLECT_STRUCT_MEMBER(obstacleAvoidanceRadius)
	REFLECT_STRUCT_MEMBER(obstacleAvoidanceHeight)
	REFLECT_STRUCT_MEMBER(obstacleAvoidanceQuality)
	REFLECT_STRUCT_MEMBER(obstacleAvoidancePriority)
	REFLECT_STRUCT_MEMBER(autoTraverseOffMesh)
	REFLECT_STRUCT_MEMBER(autoRepath)
	REFLECT_STRUCT_MEMBER(areaMask)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

static dtNavMesh* GetNavMesh() {
	return nullptr;
}

static bool GetNavMeshQuery(dtNavMeshQuery*& navMeshQuery) {
	if (navMeshQuery == nullptr) {
		return navMeshQuery->init(GetNavMesh(), 2048) == DT_SUCCESS;
	}

	return true;
}

// As components don't know about their entity, it's best to pass the startPosition to them,
// even though it isn't as simple.
bool NavAgentComponent::SetTarget(entt::entity entityHandle, glm::vec3 endPosition) {
	if (entityHandle == entt::null) {
		return false;
	}

	Grindstone::WorldContextManager* cxtMgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = cxtMgr->GetActiveWorldContextSet();
	entt::registry& registry = cxtSet->GetEntityRegistry();

	if (!registry.all_of<TransformComponent>((entt::entity)entityHandle)) {
		return false;
	}

	glm::vec3 startPosition = TransformComponent::GetWorldPosition((entt::entity)entityHandle, registry);
	destination = endPosition;

	dtQueryFilter queryFilter;
	
	glm::vec3 halfExtents(4.0f, 2.0f, 4.0f);

	if (::GetNavMeshQuery(navMeshQuery)) {
		dtPolyRef srcPolyRef = 0;
		glm::vec3 srcPoint{};
		dtPolyRef dstPolyRef = 0;
		glm::vec3 dstPoint{};

		if (navMeshQuery->findNearestPoly(&startPosition.x, &halfExtents.x, &queryFilter, &srcPolyRef, &srcPoint.x) != DT_SUCCESS) {
			return false;
		}

		if (navMeshQuery->findNearestPoly(&startPosition.x, &halfExtents.x, &queryFilter, &dstPolyRef, &dstPoint.x) != DT_SUCCESS) {
			return false;
		}

		dtStatus status = navMeshQuery->findPath(
			srcPolyRef, dstPolyRef,
			&srcPoint.x, &dstPoint.x,
			&queryFilter,
			pathPolys, &usedPolyCount, MAX_POLYS
		);

		return status == DT_SUCCESS;
	}

	return false;
}

const dtNavMeshQuery* NavAgentComponent::GetNavMeshQuery() const {
	return navMeshQuery;
}

const NavMeshLocomotionData Grindstone::Ai::NavAgentComponent::GetLocomotionData() const {
	return locomotionData;
}

extern "C" {
	AI_NAVMESH_EXPORT void* EntityGetNavmeshAgentComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = Grindstone::EngineCore::GetInstance().GetEntityRegistry();
		const entt::entity entityId = static_cast<entt::entity>(entity);
		Grindstone::Ai::NavAgentComponent* comp = reg.try_get<Grindstone::Ai::NavAgentComponent>(entityId);
		return comp;
	}

	AI_NAVMESH_EXPORT float NavmeshAgentComponentGetMaxMoveSpeed(Grindstone::Ai::NavAgentComponent& comp) {
		return comp.movementSpeed;
	}

	AI_NAVMESH_EXPORT void NavmeshAgentComponentSetMaxMoveSpeed(Grindstone::Ai::NavAgentComponent& comp, float moveSpeed) {
		comp.movementSpeed = moveSpeed;
	}

	AI_NAVMESH_EXPORT bool NavmeshAgentComponentMoveTo(Grindstone::Ai::NavAgentComponent& comp, Grindstone::Math::ExportableVector dstPosition) {
		return comp.SetTarget(entt::null, Grindstone::Math::ImportVector(dstPosition));
	}
}
