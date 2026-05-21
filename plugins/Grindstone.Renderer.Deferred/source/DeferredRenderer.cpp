#include <array>
#include <random>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/VertexArrayObject.hpp>
#include <Common/Graphics/GraphicsPipeline.hpp>
#include <Common/Event/WindowEvent.hpp>
#include <Common/Display/DisplayManager.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <Common/HashedString.hpp>

#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Materials/MaterialImporter.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/CoreComponents/EnvironmentMap/EnvironmentMapComponent.hpp>
#include <EngineCore/CoreComponents/Lights/PointLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/SpotLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Rendering/RenderGraphContextSet.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererFactory.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRenderer.hpp>

const bool shouldFastResize = true;

float lightPositions[] = {
	-1.0f, -1.0f,
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	 1.0f, -1.0f
};

uint16_t lightIndices[] = {
	0, 1, 2,
	3, 0, 2
};

DeferredRenderer::DeferredRenderer(GraphicsAPI::RenderPass* targetRenderPass) {
	if (shouldFastResize) {
		Display display = EngineCore::GetInstance().displayManager->GetMainDisplay();
		framebufferWidth = display.width;
		framebufferHeight = display.height;
		renderArea = Grindstone::Math::IntRect2D(display.width, display.height);
	}

	renderMode = DeferredRenderMode::Default;

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();

	GraphicsAPI::Buffer::CreateInfo vboCi{};
	vboCi.debugName = "Light Vertex Position Buffer";
	vboCi.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Vertex;
	vboCi.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
	vboCi.content = lightPositions;
	vboCi.bufferSize = sizeof(lightPositions);
	vertexBuffer = graphicsCore->CreateBuffer(vboCi);

	GraphicsAPI::Buffer::CreateInfo iboCi{};
	iboCi.debugName = "Light Index Buffer";
	iboCi.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Index;
	iboCi.memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
	iboCi.content = lightIndices;
	iboCi.bufferSize = sizeof(lightIndices);
	indexBuffer = graphicsCore->CreateBuffer(iboCi);

	bloom.Initialize();
	gbuffer.Initialize();
	blur.Initialize();
	ssao.Initialize();
	lighting.Initialize();
	tonemap.Initialize();
}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	graphicsCore->DeleteBuffer(vertexBuffer);
	graphicsCore->DeleteBuffer(indexBuffer);
	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);
}

bool DeferredRenderer::OnWindowResize(Events::BaseEvent* ev) {
	if (ev->GetEventType() == Events::EventType::WindowResize) {
		Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
		Resize(winResizeEvent->width, winResizeEvent->height);
	}

	return false;
}

void Grindstone::DeferredRenderer::Resize(uint32_t width, uint32_t height) {
	renderArea = Math::IntRect2D(width, height);
	framebufferWidth = width;
	framebufferHeight = height;
}

void DeferredRenderer::Render(
	GraphicsAPI::CommandBuffer* commandBuffer,
	Grindstone::WorldContextSet& worldContextSet,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderGraphBuilderResourceRef colorImageRef,
	Grindstone::Renderer::RenderGraphBuilderResourceRef depthImageRef
) {
	entt::registry& registry = worldContextSet.GetEntityRegistry();
	Grindstone::Rendering::RenderGraphWorldContext* renderingContext = static_cast<Grindstone::Rendering::RenderGraphWorldContext*>(
		worldContextSet.GetContext(Grindstone::Rendering::renderGraphWorldContextName)
	);

	GS_ASSERT(renderingContext != nullptr);

	Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::AssetRendererManager* assetManager = engineCore.assetRendererManager;
	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

	uint32_t imageIndex = wgb->GetCurrentImageIndex();

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea,
	};

	// TODO: Move these into the ssao pass, maybe?
	Grindstone::Renderer::ImageDescription attachmentAmbientOcclusionBlur{
		.name = "Ambient Occlusion Blurred",
		.format = ambientOcclusionFormat,
		.imageUsage = Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget | Grindstone::GraphicsAPI::ImageUsageFlags::Sampled
	};

	Grindstone::Renderer::MetaRect ssaoBlurMetaRect(Renderer::MetaSize2D::Zero(), Renderer::MetaSize2D::DivideSwapchain(2));

	Grindstone::Renderer::ShadowPassReturnData shadowOutput = shadows.AddShadowPasses(renderGraphBuilder, worldContextSet);
	Grindstone::Renderer::GbufferData gbufferData = gbuffer.AddPass(depthImageRef, projectionMatrix, viewMatrix, renderGraphBuilder);
	Grindstone::Renderer::RenderGraphBuilderResourceRef ssaoOutput = ssao.AddPass(vertexBuffer, indexBuffer, renderGraphBuilder, gbufferData);
	// TODO: Move this into the ssao pass, maybe? Specify a Two-Pass Separable Blur
	Grindstone::Renderer::RenderGraphBuilderResourceRef ssaoBlurredOutput = blur.AddPass(renderGraphBuilder, ssaoBlurMetaRect, attachmentAmbientOcclusionBlur, ssaoOutput);
	Grindstone::Renderer::LightingPassReturnData lightingData = lighting.AddPass(vertexBuffer, indexBuffer, renderGraphBuilder, gbufferData, shadowOutput.shadowOutputRef, ssaoBlurredOutput);
	// auto ssrOutput = ssr.AddPass(renderGraph, lightingOutput);
	// auto dofOutput = dof.AddPass(renderGraph, ssrOutput);

	Grindstone::Renderer::RenderGraphBuilderResourceRef bloomOutput = bloom.AddBloomChain(
		imageIndex,
		Grindstone::Math::Uint2(renderArea.extent.x, renderArea.extent.y),
		renderGraphBuilder,
		lightingData.lightingOutputRef
	);
	
	if (renderMode == DeferredRenderMode::Default) {
		Grindstone::Renderer::TonemapPassReturnData data = tonemap.AddPass(renderGraphBuilder, {}, lightingData.lightingOutputRef, bloomOutput, colorImageRef);
	}
	/*
	else {
		debug.AddPass(renderGraph, gbufferOutput, lightingOutput);
	}
	*/
}

uint16_t DeferredRenderer::GetRenderModeCount() const {
	Grindstone::DeferredRendererFactory* factory = static_cast<Grindstone::DeferredRendererFactory*>(EngineCore::GetInstance().GetRendererFactory());
	return factory->GetRenderModeCount();
}

const Grindstone::BaseRenderer::RenderMode* DeferredRenderer::GetRenderModes() const {
	Grindstone::DeferredRendererFactory* factory = static_cast<Grindstone::DeferredRendererFactory*>(EngineCore::GetInstance().GetRendererFactory());
	return factory->GetRenderModes();
}

void DeferredRenderer::SetRenderMode(uint16_t mode) {
	renderMode = static_cast<DeferredRenderMode>(mode);
}

std::vector<Grindstone::Rendering::GeometryRenderStats> Grindstone::DeferredRenderer::GetRenderingStats() {
	Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t imageIndex = wgb->GetCurrentImageIndex();

	return {};
}
