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

	Grindstone::GraphicsAPI::Sampler::CreateInfo screenSamplerCreateInfo{};
	screenSamplerCreateInfo.debugName = "Screen Sampler";
	screenSamplerCreateInfo.options.anistropy = 0;
	screenSamplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Linear;
	screenSamplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Linear;
	screenSamplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat;
	screenSamplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat;
	screenSamplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat;
	// TODO: Use this Sampler
	auto screenSampler = graphicsCore->CreateSampler(screenSamplerCreateInfo);

}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	graphicsCore->DeleteBuffer(gpuGlobalUniformBufferObject);

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

void DeferredRenderer::Render(
	GraphicsAPI::CommandBuffer* commandBuffer,
	Grindstone::WorldContextSet& worldContextSet,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::RenderAttachment& outRenderAttachment,
	Grindstone::GraphicsAPI::Image* depthTarget
) {
	entt::registry& registry = worldContextSet.GetEntityRegistry();
	Grindstone::Rendering::RenderGraphWorldContext* renderingContext = static_cast<Grindstone::Rendering::RenderGraphWorldContext*>(
		worldContextSet.GetContext(Grindstone::Rendering::renderGraphWorldContextName)
	);

	GS_ASSERT(renderingContext != nullptr);

	Grindstone::Renderer::RenderGraph& renderGraph = renderingContext->GetRenderGraph();

	Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::AssetRendererManager* assetManager = engineCore.assetRendererManager;
	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

	graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

	uint32_t imageIndex = wgb->GetCurrentImageIndex();

	EngineUboStruct engineUboStruct{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.inverseProjectionMatrix = glm::inverse(projectionMatrix),
		.inverseViewMatrix = glm::inverse(viewMatrix),
		.eyePos = eyePos,
		.framebufferResolution = glm::vec2(framebufferWidth, framebufferHeight),
		.renderResolution = glm::vec2(renderArea.GetWidth(), renderArea.GetHeight()),
		.renderScale = glm::vec2(static_cast<float>(renderArea.GetWidth()) / framebufferWidth, static_cast<float>(renderArea.GetHeight()) / framebufferHeight),
		.time = static_cast<float>(engineCore.GetTimeSinceLaunch())
	};

	// globalUniformBufferObject->UploadData(&engineUboStruct);
	// commandBuffer->CopyBufferRegion(imageSet.globalUniformBufferObject, gpuGlobalUniformBufferObject);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea,
	};

	// assetManager->SetEngineDescriptorSet(imageSet.engineDescriptorSet);

	// auto shadowOutput = shadows.AddPass(renderGraph);
	gbuffer.AddPass(projectionMatrix, viewMatrix, renderGraph);
	// auto ssaoOutput = ssao.AddPass(renderGraph, gbufferOutput);
	// auto ssaoBlurredOutput = blur.AddTwoPassBlur(renderGraph, ssaoOutput);
	lighting.AddPass(vertexBuffer, indexBuffer, renderGraph);
	// auto ssrOutput = ssr.AddPass(renderGraph, lightingOutput);
	// auto dofOutput = dof.AddPass(renderGraph, ssrOutput);
	// auto bloomOutput = bloom.AddBloomChain(renderGraph, dofOutput);

	if (renderMode == DeferredRenderMode::Default) {
		tonemap.AddPass(renderGraph);
	}
	/*
	else {
		debug.AddPass(renderGraph, gbufferOutput, lightingOutput);
	}
	*/

	Grindstone::Renderer::RenderGraph::RenderGraphContext context{
		.swapchainSize = renderArea.extent,
		.commandBuffer = commandBuffer,
		.worldContextSet = &worldContextSet,
	};

	renderGraph.ExecuteGraph(context);
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
