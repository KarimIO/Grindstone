#pragma once

#include <glm/glm.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include <entt/entt.hpp>

namespace Grindstone {
	class BaseRenderer;

	namespace Events {
		struct BaseEvent;
	}

	struct CameraComponent {
		bool isOrthographic = false;
		float nearPlaneDistance = 0.1f;
		float farPlaneDistance = 200.0f;
		float fieldOfView = glm::radians(90.0f);
		float aspectRatio = 800.0f / 600.0f;
		BaseRenderer* renderer = nullptr;

		bool OnWindowResize(Events::BaseEvent* ev);

		REFLECT("Camera")
	};

	void SetupCameraComponent(entt::registry& registry, entt::entity entity, void* componentPtr);
}
