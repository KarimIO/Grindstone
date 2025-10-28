#pragma once

#include "../NavAgentType.hpp"
#include "../NavMeshAreaTypes.hpp"

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

		REFLECT("NavigationAgent")
	};
}
