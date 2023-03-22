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
			return Math::Float3(
				2 * (rotation.x * rotation.z + rotation.w * rotation.y),
				2 * (rotation.y * rotation.z - rotation.w * rotation.x),
				1 - 2 * (rotation.x * rotation.x + rotation.y * rotation.y)
			);
		}

		Math::Float3 GetRight() {
			return Math::Float3(
				1 - 2 * (rotation.y * rotation.y + rotation.z * rotation.z),
				2 * (rotation.x * rotation.y + rotation.w * rotation.z),
				2 * (rotation.x * rotation.z - rotation.w * rotation.y)
			);
		}

		Math::Float3 GetUp() {
			return Math::Float3(
				2 * (rotation.x * rotation.y - rotation.w * rotation.z),
				1 - 2 * (rotation.x * rotation.x + rotation.z * rotation.z),
				2 * (rotation.y * rotation.z + rotation.w * rotation.x)
			);
		}

		REFLECT("Transform")
	};
}
