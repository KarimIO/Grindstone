#include <chrono>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <Common/Graphics/Buffer.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/CoreComponents/Camera/CameraComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/Rendering/RenderingPipeline.hpp>

#include "RenderSystem.hpp"

std::vector<Grindstone::GraphicsAPI::CommandBuffer*> commandBuffers;
static std::array<Grindstone::Renderer::TransientResourceManager*, 3> transientResourceManagers{};
Grindstone::GraphicsAPI::Buffer* gpuGlobalUniformBufferObject = nullptr;
Grindstone::GraphicsAPI::DescriptorSetLayout* globalDescriptorSetLayout;
Grindstone::GraphicsAPI::Buffer* globalStagingUniformBufferObject;
std::array<Grindstone::GraphicsAPI::Buffer*, 3> globalUniformBufferObject;
std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> globalDescriptorSet;

struct EngineUboStruct {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 inverseProjectionMatrix;
	glm::mat4 inverseViewMatrix;
	glm::vec3 eyePos;
	float alignmentBufferForPreviousVec3;
	glm::vec2 framebufferResolution;
	glm::vec2 renderResolution;
	glm::vec2 renderScale;
	float time;
};

static void SetupGlobalUbos() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	Grindstone::GraphicsAPI::Buffer::CreateInfo globalUboCreateInfo{
		.content = nullptr,
		.bufferSize = sizeof(EngineUboStruct),
		.bufferUsage =
			Grindstone::GraphicsAPI::BufferUsage::TransferDst |
			Grindstone::GraphicsAPI::BufferUsage::TransferSrc |
			Grindstone::GraphicsAPI::BufferUsage::Uniform,
		.memoryUsage = Grindstone::GraphicsAPI::MemoryUsage::CPUToGPU,
	};

	Grindstone::GraphicsAPI::DescriptorSetLayout::Binding globalDescriptorSetLayoutBinding{
		.bindingId = 0,
		.count = 1,
		.type = Grindstone::GraphicsAPI::BindingType::UniformBuffer,
		.stages = Grindstone::GraphicsAPI::ShaderStageBit::All,
	};

	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo globalDescriptorSetLayoutCreateInfo{
		.debugName = "Global UBO Descriptor Set Layout",
		.bindings = &globalDescriptorSetLayoutBinding,
		.bindingCount = 1u,
	};

	globalDescriptorSetLayout = graphicsCore->GetOrCreateDescriptorSetLayoutFromCache(globalDescriptorSetLayoutCreateInfo);

	Grindstone::GraphicsAPI::DescriptorSet::CreateInfo globalDescriptorSetsCreateInfo{
		.layout = globalDescriptorSetLayout,
		.bindingCount = 1u
	};

	globalUboCreateInfo.debugName = "Global Staging UBO";
	globalStagingUniformBufferObject = graphicsCore->CreateBuffer(globalUboCreateInfo);

	for (size_t i = 0; i < 3; ++i) {
		std::string uboDebugName = std::vformat("Global UBO [{}]", std::make_format_args(i));
		globalUboCreateInfo.debugName = uboDebugName.c_str();
		globalUniformBufferObject[i] = graphicsCore->CreateBuffer(globalUboCreateInfo);

		std::string descriptorSetDebugName = std::vformat("Global UBO [{}]", std::make_format_args(i));
		Grindstone::GraphicsAPI::DescriptorSet::Binding binding = Grindstone::GraphicsAPI::DescriptorSet::Binding::UniformBuffer(globalUniformBufferObject[i]);
		globalDescriptorSetsCreateInfo.bindings = &binding;
		globalDescriptorSetsCreateInfo.debugName = descriptorSetDebugName.c_str();
		globalDescriptorSet[i] = graphicsCore->CreateDescriptorSet(globalDescriptorSetsCreateInfo);
	}
}

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
			SetupGlobalUbos();

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
				Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
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

				glm::mat4 adjustedPerspectiveMatrix = projectionMatrix;
				graphicsCore->AdjustPerspective( &adjustedPerspectiveMatrix[0][0]);

				glm::vec2 renderResolution = glm::vec2(width, height);

				EngineUboStruct engineUboStruct{
					.projectionMatrix = adjustedPerspectiveMatrix,
					.viewMatrix = viewMatrix,
					.inverseProjectionMatrix = glm::inverse(adjustedPerspectiveMatrix),
					.inverseViewMatrix = glm::inverse(viewMatrix),
					.eyePos = pos,
					.framebufferResolution = renderResolution,
					.renderResolution = renderResolution,
					.renderScale = glm::vec2(1.0f, 1.0f),
					.time = static_cast<float>(engineCore.GetTimeSinceLaunch())};

				globalStagingUniformBufferObject->UploadData(&engineUboStruct);
				currentCommandBuffer->CopyBufferRegion(
					globalStagingUniformBufferObject,
					globalUniformBufferObject[swapchainIndex]);

				GraphicsAPI::RenderPass* renderPass = wgb->GetRenderPass();
				GraphicsAPI::Framebuffer* framebuffer = wgb->GetCurrentFramebuffer();

				GraphicsAPI::ClearColor clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

				Grindstone::Math::IntRect2D viewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());

				Grindstone::GraphicsAPI::Image* image = wgb->GetCurrentSwapchainImage();
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
					.globalDescriptorSetLayout = globalDescriptorSetLayout,
					.globalDescriptorSet = globalDescriptorSet[swapchainIndex],
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

				Renderer::RenderGraphBuilderResourceRef depthImageRef = renderGraphBuilder.AddImage(
					Grindstone::Renderer::ImageDescription{
						.name = "Depth Image",
						.size = Grindstone::Renderer::MetaSize2D::Viewport(),
						.samples = 1,
						.mipLevels = 1,
						.depth = 1,
						.arrayLayers = 1,
						.format = Grindstone::GraphicsAPI::Format::D32_SFLOAT,
						.imageDimensions = GraphicsAPI::ImageDimension::Dimension2D,
						.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly,
						.imageUsage = GraphicsAPI::ImageUsageFlags::DepthStencil | GraphicsAPI::ImageUsageFlags::Sampled,
						.externalGetterCallback = nullptr
					}
				);

				Grindstone::Renderer::RenderFrameContext renderFrameContext(worldContextSet);
				renderFrameContext.imageIndex = wgb->GetCurrentSwapchainIndex();
				renderFrameContext.imageSize = {width, height};
				renderFrameContext.colorRef = colorImageRef;
				renderFrameContext.depthRef = depthImageRef;
				renderFrameContext.views.emplace_back(
					Renderer::RenderFrameViewContext{
						.projectionMatrix = projectionMatrix,
						.viewMatrix = viewMatrix,
						.projectionViewMatrix = projectionMatrix * viewMatrix,
						.inverseProjectionMatrix = glm::inverse(projectionMatrix),
						.inverseViewMatrix = glm::inverse(viewMatrix),
						.inverseProjectionViewMatrix = glm::inverse(projectionMatrix * viewMatrix),
						.eyePos = pos,
						.nearDistance = cameraComponent.nearPlaneDistance,
						.farDistance = cameraComponent.farPlaneDistance
					}
				);


				renderPipeline->Render(renderGraphBuilder, renderFrameContext);

				auto renderGraph = renderGraphBuilder.Compile();
				renderGraph.ExecuteGraph(context);

				currentCommandBuffer->EndCommandBuffer();
				wgb->SubmitCommandBufferForCurrentFrame(currentCommandBuffer);

				if (cameraComponent.isMainCamera) {
					wgb->PresentSwapchain();
				}
			}
		);
	}
}
