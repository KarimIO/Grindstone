#pragma once

#include <glm/glm.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	class BaseRenderer;
	class WorldContextSet;

	namespace Events {
		struct BaseEvent;
	}

	struct CameraComponent {
		bool isMainCamera = true;
		bool isOrthographic = false;
		float nearPlaneDistance = 0.1f;
		float farPlaneDistance = 200.0f;
		float fieldOfView = glm::radians(90.0f);
		float aspectRatio = 800.0f / 600.0f;
		BaseRenderer* renderer = nullptr;

		bool OnWindowResize(Events::BaseEvent* ev);

		REFLECT("Camera")
	};

	void SetupCameraComponent(Grindstone::WorldContextSet&, entt::entity);
	void DestroyCameraComponent(Grindstone::WorldContextSet&, entt::entity);
}
