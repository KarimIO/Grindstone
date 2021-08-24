#pragma once

#include <glm/gtx/quaternion.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct TransformComponent {
		glm::quat rotation;
		Math::Float3 position;
		Math::Float3 scale;

		REFLECT("Transform")
	};
}
