#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	class BaseRenderer;

	struct CameraComponent {
		bool isOrthographic = false;
		float near = 0.1f;
		float far = 100.0f;
		float fov = 90.0f;
		float aspectRatio = 1.0f;
		BaseRenderer* renderer = nullptr;

		REFLECT("Camera")
	};
}
