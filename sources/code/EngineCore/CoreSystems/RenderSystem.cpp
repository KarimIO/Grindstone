#include <iostream>
#include <chrono>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "RenderSystem.hpp"
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
	void RenderSystem(entt::registry& registry) {
		EngineCore& engineCore = EngineCore::GetInstance();
		const auto upVector = glm::vec3(0, 1, 0);
		auto view = registry.view<const TransformComponent, const CameraComponent>();

		auto duration = std::chrono::system_clock::now().time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		double time = millis / 1000.0;

		view.each([&](
			const TransformComponent& transformComponent,
			const CameraComponent& cameraComponent
		) {
			// const glm::vec3 forwardVector = eulerToForward(transformComponent.angles);
			// const glm::vec3 pos = transformComponent.position;
			const glm::vec3 pos = glm::vec3(
				12 * glm::cos(time * 2),
				3,
				3 * glm::sin(time * 2)
			);
			//transformComponent.position;

			const auto viewMatrix = glm::lookAt(
				pos,
				glm::vec3(),
				upVector
			);

			const auto projectionMatrix = glm::perspective(
				cameraComponent.fov,
				cameraComponent.aspectRatio,
				cameraComponent.near,
				cameraComponent.far
			);

			cameraComponent.renderer->Render(
				registry,
				projectionMatrix,
				viewMatrix,
				pos,
				nullptr
			);
		});
	}
}
