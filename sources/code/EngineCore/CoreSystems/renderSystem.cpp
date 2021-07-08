#include <iostream>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderSystem.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "EngineCore/CoreComponents/Camera/CameraComponent.hpp"
#include "EngineCore/Rendering/BaseRenderer.hpp"
#include "EngineCore/EngineCore.hpp"

glm::vec3 eulerToForward(glm::vec3 eulerAngle) {
	float pitch = eulerAngle.x;
	float yaw = eulerAngle.y;

	glm::vec3 forwardVector;
	forwardVector.x = cos(yaw) * cos(pitch);
	forwardVector.y = sin(pitch);
	forwardVector.z = sin(yaw) * cos(pitch);
	return glm::normalize(forwardVector);
}

namespace Grindstone {
	void renderSystem(EngineCore* engineCore, entt::registry& registry) {
		const auto upVector = glm::vec3(0, 1, 0);
		auto view = registry.view<const TransformComponent, const CameraComponent>();

		view.each([&](
			const TransformComponent& transformComponent,
			const CameraComponent& cameraComponent
		) {
			const glm::vec3 forwardVector = eulerToForward(transformComponent.angles);

			const auto viewMatrix = glm::lookAt(
				transformComponent.position,
				forwardVector,
				upVector
			);

			const auto projectionMatrix = glm::perspective(
				cameraComponent.fov,
				cameraComponent.aspectRatio,
				cameraComponent.near,
				cameraComponent.far
			);

			BaseRender(
				engineCore->getGraphicsCore(),
				projectionMatrix,
				viewMatrix
			);
		});
	}
}
