#include <iostream>
#include <chrono>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Common/Window/WindowManager.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/Rendering/RenderingPipeline.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Profiling.hpp>

#include "RenderSystem.hpp"

std::vector<Grindstone::GraphicsAPI::CommandBuffer*> commandBuffers;
static std::array<Grindstone::Renderer::TransientResourceManager*, 3> transientResourceManagers{};

namespace Grindstone {
	void RenderSystem(Grindstone::WorldContextSet& worldContextSet) {
		GRIND_PROFILE_SCOPE("RenderSystem()");

		EngineCore& engineCore = EngineCore::GetInstance();
		if (engineCore.isEditor) {
			return;
		}

		GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
		Grindstone::Renderer::RenderingPipeline* renderPipeline = engineCore.GetRenderingPipeline();
		GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

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

		// TODO: We don't always want to render to the entire window, we don't always want to render to window 0.
		// We need a way to figure out what viewport and window to draw to, and therefore what wgb.

		entt::registry& registry = worldContextSet.GetEntityRegistry();
		auto view = registry.view<entt::entity, const TransformComponent, const CameraComponent>();

		view.each(
			[&worldContextSet, renderPipeline, graphicsCore, wgb](
				entt::entity entity,
				const TransformComponent& transformComponent,
				const CameraComponent& cameraComponent
			) {
				uint32_t width = wgb->GetCurrentFramebuffer()->GetWidth();
				uint32_t height = wgb->GetCurrentFramebuffer()->GetHeight();
				uint32_t swapchainIndex = wgb->GetCurrentImageIndex();

				// TODO: Handle case for not main camera
				// Do we only want to allow rendering to backbuffer and render targets
				GraphicsAPI::CommandBuffer* currentCommandBuffer = cameraComponent.isMainCamera
					? commandBuffers[swapchainIndex]
					: nullptr;

				if (currentCommandBuffer == nullptr) {
					return;
				}

				currentCommandBuffer->BeginCommandBuffer();

				if (std::isnan(cameraComponent.aspectRatio)) {
					return;
				}

				entt::registry& registry = worldContextSet.GetEntityRegistry();
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

				Grindstone::GraphicsAPI::Image* image = wgb->GetCurrentFramebuffer()->GetRenderTarget(0);
				Grindstone::GraphicsAPI::RenderAttachment attachment{
					.image = image,
					.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
					.clearValue = Grindstone::GraphicsAPI::ClearColor()
				};

				Grindstone::Renderer::TransientResourceManager*& transientResourceManager = transientResourceManagers[swapchainIndex];
				if (transientResourceManager == nullptr) {
					transientResourceManager = Grindstone::Memory::AllocatorCore::Allocate<Grindstone::Renderer::TransientResourceManager>();
				}

				Grindstone::Renderer::RenderGraphContext context{
					.graphicsCore = graphicsCore,
					.cameraViewData = Grindstone::Rendering::RenderViewData {
						.projectionMatrix = projectionMatrix,
						.viewMatrix = viewMatrix,
						.renderArea = Math::IntRect2D(width, height),
					},
					.transientResourceManager = transientResourceManager,
					// TODO: RenderGraph2.0 Set GlobalDescriptorSets
					// .globalDescriptorSetLayout = globalDescriptorSetLayout,
					// .globalDescriptorSet = globalDescriptorSet[imageIndex],
					.swapchainSize = Math::Extent2D(width, height),
					.commandBuffer = currentCommandBuffer,
					.worldContextSet = &worldContextSet,
					.swapchainIndex = swapchainIndex
				};

				Grindstone::Renderer::RenderGraphBuilder renderGraphBuilder;

				Renderer::RenderGraphBuilderResourceRef colorImageRef = renderGraphBuilder.AddImage(
					Grindstone::Renderer::ImageDescription{
						.name = "Camera Output Image (Tonemapped)",
						.size = Grindstone::Renderer::MetaSize2D::Viewport(),
						.samples = 1,
						.mipLevels = 1,
						.depth = 1,
						.arrayLayers = 1,
						.format = Grindstone::GraphicsAPI::Format::R8G8B8A8_SNORM,
						.imageDimensions = GraphicsAPI::ImageDimension::Dimension2D,
						.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly,
						.imageUsage = GraphicsAPI::ImageUsageFlags::RenderTarget | GraphicsAPI::ImageUsageFlags::Sampled,
						.externalInitialLayout = GraphicsAPI::ImageLayout::Undefined,
						.externalInitialAccessFlags = GraphicsAPI::AccessFlags::None,
						.externalInitialPipelineStage = GraphicsAPI::PipelineStageBit::TopOfPipe,
						.externalFinalLayout = GraphicsAPI::ImageLayout::ShaderRead,
						.externalFinalAccessFlags = GraphicsAPI::AccessFlags::ShaderRead,
						.externalFinalPipelineStage = GraphicsAPI::PipelineStageBit::FragmentShader,
						.externalGetterCallback = [image]() { return image; }
					}
				);

				Grindstone::Renderer::RenderFrameContext renderFrameContext(worldContextSet);

				renderPipeline->Render(renderGraphBuilder, renderFrameContext);

				currentCommandBuffer->EndCommandBuffer();
				wgb->SubmitCommandBufferForCurrentFrame(currentCommandBuffer);

				if (cameraComponent.isMainCamera) {
					wgb->PresentSwapchain();
				}
			}
		);
	}
}
