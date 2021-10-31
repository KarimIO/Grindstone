#pragma once

#include <glm/glm.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	class BaseRenderer;

	namespace Events {
		struct BaseEvent;
	}

	struct CameraComponent {
		bool isOrthographic = false;
		float near = 0.1f;
		float far = 200.0f;
		float fov = glm::radians(90.0f);
		float aspectRatio = 800.0f / 600.0f;
		BaseRenderer* renderer = nullptr;

		bool OnWindowResize(Events::BaseEvent* ev);

		REFLECT("Camera")
	};
}
