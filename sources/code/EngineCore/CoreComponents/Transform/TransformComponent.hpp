#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct TransformComponent {
		Math::Float3 position;
		Math::Float3 angles;
		Math::Float3 scale;

		REFLECT("Transform")
	};
}
