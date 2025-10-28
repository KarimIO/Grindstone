#pragma once

#include <glm/glm.hpp>
#include "../NavMeshAreaTypes.hpp"

namespace Grindstone::Ai {
	struct OffNavMeshConnectionComponent {
		enum class Direction : uint8_t {
			AToB,
			Bidirectional,
		};

		glm::vec3 positionA;
		glm::vec3 positionB;
		float radius;
		Direction direction;
		NavAreaId areaId;
		NavAreaFlags flags;

		REFLECT("OffNavMeshConnection")
	};
}
