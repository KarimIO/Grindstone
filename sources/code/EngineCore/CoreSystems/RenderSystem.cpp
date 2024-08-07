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
#include <EngineCore/Profiling.hpp>

#include "RenderSystem.hpp"

std::vector<Grindstone::GraphicsAPI::CommandBuffer*> commandBuffers;

namespace Grindstone {
	void RenderSystem(entt::registry& registry) {
		GRIND_PROFILE_SCOPE("RenderSystem()");

		EngineCore& engineCore = EngineCore::GetInstance();
		if (engineCore.isEditor) {
			return;
		}

		GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

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

		const static glm::vec3 upVector = glm::vec3(0, 1, 0);
		auto view = registry.view<const TransformComponent, const CameraComponent>();

		view.each(
			[&](
				const TransformComponent& transformComponent,
				const CameraComponent& cameraComponent
			) {
				// TODO: Handle case for not main camera
				// Do we only want to allow rendering to backbuffer and render targets, or also allow useless framebuffers
				GraphicsAPI::CommandBuffer* currentCommandBuffer = cameraComponent.isMainCamera
					? commandBuffers[wgb->GetCurrentImageIndex()]
					: nullptr;

				if (currentCommandBuffer == nullptr) {
					return;
				}

				currentCommandBuffer->BeginCommandBuffer();

				if (std::isnan(cameraComponent.aspectRatio) || cameraComponent.renderer == nullptr) {
					return;
				}

				const glm::vec3 forwardVector = transformComponent.GetForward();
				const glm::vec3 pos = transformComponent.position;

				const glm::mat4 viewMatrix = glm::lookAt(
					pos,
					pos + forwardVector,
					upVector
				);

				const glm::mat4 projectionMatrix = glm::perspective(
					cameraComponent.fieldOfView,
					cameraComponent.aspectRatio,
					cameraComponent.nearPlaneDistance,
					cameraComponent.farPlaneDistance
				);

				cameraComponent.renderer->Render(
					currentCommandBuffer,
					registry,
					projectionMatrix,
					viewMatrix,
					pos,
					wgb->GetCurrentFramebuffer()
				);

				currentCommandBuffer->EndCommandBuffer();
				wgb->SubmitCommandBuffer(currentCommandBuffer);

				if (cameraComponent.isMainCamera) {
					wgb->PresentSwapchain();
				}
			}
		);
	}
}
