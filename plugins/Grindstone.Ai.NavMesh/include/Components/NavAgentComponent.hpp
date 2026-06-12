#pragma once

#include <entt/entity/handle.hpp>

#include <Grindstone.Ai.NavMesh/include/NavAgentType.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshAreaTypes.hpp>
#include <Grindstone.Ai.NavMesh/include/NavAgentLocomotionData.hpp>
#include <DetourNavMesh.h>

class dtNavMeshQuery;

namespace Grindstone::Ai {
	struct NavAgentComponent {
		NavAgentTypeId typeId;
		float baseOffset;
		float movementSpeed;
		float angularSpeed;
		float acceleration;
		float stoppingDistance;
		bool autoBraking;
		float obstacleAvoidanceRadius;
		float obstacleAvoidanceHeight;
		float obstacleAvoidanceQuality;
		float obstacleAvoidancePriority;
		bool autoTraverseOffMesh;
		bool autoRepath;
		NavAreaId areaMask; // one bit per area type

		bool SetTarget(entt::entity entityHandle, glm::vec3 endPosition);
		const dtNavMeshQuery* GetNavMeshQuery() const;
		const NavMeshLocomotionData GetLocomotionData() const;
		glm::vec3 destination;
		static const int MAX_POLYS = 256;

	protected:
		dtNavMeshQuery* navMeshQuery = nullptr;
		NavMeshLocomotionData locomotionData;
		dtPolyRef pathPolys[MAX_POLYS];
		int usedPolyCount = 0;

		REFLECT("NavigationAgent")
	};
}
