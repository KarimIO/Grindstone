#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct TransformComponent {
		Math::Vec3 position;
		Math::Vec3 angles;
		Math::Vec3 scale;

		REFLECT("Transform")
	};
}
