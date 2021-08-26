#pragma once

#include <glm/gtx/quaternion.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct TransformComponent {
		glm::quat rotation;
		Math::Float3 position;
		Math::Float3 scale = Math::Float3(1.f,1.f,1.f);

		REFLECT("Transform")
	};
}
