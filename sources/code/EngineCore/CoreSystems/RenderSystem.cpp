#include <iostream>
#include <chrono>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
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

		uint32_t width = wgb->GetCurrentFramebuffer()->GetWidth();
		uint32_t height = wgb->GetCurrentFramebuffer()->GetHeight();

		if (!wgb->AcquireNextImage()) {
			return;
		}

		if (commandBuffers.size() == 0) {
			commandBuffers.resize(wgb->GetMaxFramesInFlight());
			GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

			for (size_t i = 0; i < commandBuffers.size(); ++i) {
				std::string debugName = "Main Command Buffer " + std::to_string(i);
				commandBufferCreateInfo.debugName = debugName.c_str();

				commandBuffers[i] = engineCore.GetGraphicsCore()->CreateCommandBuffer(commandBufferCreateInfo);
			}
		}

		auto view = registry.view<entt::entity, const TransformComponent, const CameraComponent>();

		view.each(
			[&](
				entt::entity entity,
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

				const glm::mat4 transformMatrix = TransformComponent::GetWorldTransformMatrix(entity, registry);
				const glm::vec3 upVector = glm::normalize(-glm::vec3(transformMatrix[1]));
				const glm::vec3 forwardVector = glm::normalize(glm::vec3(transformMatrix[2]));
				const glm::vec3 pos = glm::vec3(transformMatrix[3]);

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

				if (cameraComponent.isMainCamera) {
					cameraComponent.renderer->Resize(width, height);
				}

				cameraComponent.renderer->Render(
					currentCommandBuffer,
					registry,
					projectionMatrix,
					viewMatrix,
					pos,
					wgb->GetCurrentFramebuffer()
				);

				currentCommandBuffer->EndCommandBuffer();
				wgb->SubmitCommandBufferForCurrentFrame(currentCommandBuffer);

				if (cameraComponent.isMainCamera) {
					wgb->PresentSwapchain();
				}
			}
		);
	}
}
