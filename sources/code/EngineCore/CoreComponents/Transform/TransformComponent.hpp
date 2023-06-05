#pragma once

#include <glm/gtx/quaternion.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	struct TransformComponent {
		Math::Quaternion rotation;
		Math::Float3 position = Math::Float3(0.0f, 0.0f, 0.0f);
		Math::Float3 scale = Math::Float3(1.f, 1.f, 1.f);

		Math::Float3 GetForward() {
			return rotation * Math::Float3(0.0f, 0.0f,-1.0f);
		}

		Math::Float3 GetRight() {
			return rotation * Math::Float3(1.0f, 0.0f, 0.0f);
		}

		Math::Float3 GetUp() {
			return rotation * Math::Float3(0.0f, 1.0f, 0.0f);
		}

		REFLECT("Transform")
	};
}
