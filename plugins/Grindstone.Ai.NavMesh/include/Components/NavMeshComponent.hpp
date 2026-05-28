#pragma once

#include "../NavMeshAreaTypes.hpp"

class dtNavMesh;

namespace Grindstone::Ai {
	struct NavMeshComponent {
		float cellSize = 0.2f;
		float cellHeight = 0.1f;
		dtNavMesh* navMesh = nullptr;

		REFLECT("NavigationMesh")
	};
}
