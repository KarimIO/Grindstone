#pragma once

#include "../NavMeshAreaTypes.hpp"

namespace Grindstone::Ai {
	struct NavMeshComponent {
		float cellSize = 0.2f;
		float cellHeight = 0.1f;

		REFLECT("NavigationMesh")
	};
}
