#include <iostream>
#include <chrono>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>
#include <EngineCore/EngineCore.hpp>
#include "RenderSystem.hpp"

glm::vec3 EulerToForward(glm::vec3 eulerAngle) {
	float pitch = eulerAngle.x;
	float yaw = eulerAngle.y;

	glm::vec3 forwardVector;
	forwardVector.x = cos(yaw) * cos(pitch);
	forwardVector.y = sin(pitch);
	forwardVector.z = sin(yaw) * cos(pitch);
	return glm::normalize(forwardVector);
}

std::vector<Grindstone::GraphicsAPI::CommandBuffer*> commandBuffers;

namespace Grindstone {
	void RenderSystem(entt::registry& registry) {
		EngineCore& engineCore = EngineCore::GetInstance();
		auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
		if (!wgb->AcquireNextImage()) {
			return;
		}

		if (commandBuffers.size() == 0) {
			commandBuffers.resize(wgb->GetMaxFramesInFlight());
			GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

			for (size_t i = 0; i < commandBuffers.size(); ++i) {
				commandBuffers[i] = engineCore.GetGraphicsCore()->CreateCommandBuffer(commandBufferCreateInfo);
			}
		}

		auto currentCommandBuffer = commandBuffers[wgb->GetCurrentImageIndex()];
		currentCommandBuffer->BeginCommandBuffer();

		const auto upVector = glm::vec3(0, 1, 0);
		auto view = registry.view<const TransformComponent, const CameraComponent>();

		auto duration = std::chrono::system_clock::now().time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		double time = millis / 1000.0;

		view.each(
			[&](
				const TransformComponent& transformComponent,
				const CameraComponent& cameraComponent
			) {
				if (std::isnan(cameraComponent.aspectRatio) || cameraComponent.renderer == nullptr) {
					return;
				}

				const glm::vec3 forwardVector = transformComponent.rotation * Math::Float3(0.0f, 0.0f, -1.0f);
				const glm::vec3 pos = transformComponent.position;

				const auto viewMatrix = glm::lookAt(
					pos,
					pos + forwardVector,
					upVector
				);

				auto projectionMatrix = glm::perspective(
					cameraComponent.fieldOfView,
					cameraComponent.aspectRatio,
					cameraComponent.nearPlaneDistance,
					cameraComponent.farPlaneDistance
				);

				engineCore.GetGraphicsCore()->AdjustPerspective(&projectionMatrix[0][0]);

				cameraComponent.renderer->Render(
					currentCommandBuffer,
					registry,
					projectionMatrix,
					viewMatrix,
					pos,
					wgb->GetCurrentFramebuffer()
				);
			}
		);

		currentCommandBuffer->EndCommandBuffer();
		wgb->SubmitCommandBuffer(currentCommandBuffer);
		wgb->PresentSwapchain();
	}
}
