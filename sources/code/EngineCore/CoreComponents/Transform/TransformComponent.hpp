#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	struct TransformComponent {
		float position[3];
		float angles[3];
		float scale[3];

		float world[4][4];

		REFLECT("Transform")
	};
}
