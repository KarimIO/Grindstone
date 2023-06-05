#include <array>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/VertexArrayObject.hpp"
#include "Common/Graphics/Pipeline.hpp"
#include "DeferredRenderer.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "EngineCore/CoreComponents/Lights/PointLightComponent.hpp"
#include "EngineCore/AssetRenderer/AssetRendererManager.hpp"
#include "Common/Event/WindowEvent.hpp"
#include "EngineCore/Profiling.hpp"
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

GraphicsAPI::RenderPass* DeferredRenderer::gbufferRenderPass = nullptr;

float lightPositions[] = {
	-1.0f, -1.0f,
	-1.0f,  1.0f,
	 1.0f,  1.0f,
	 1.0f, -1.0f
};

uint16_t lightIndices[] = {
	0, 1, 2,
	3, 2, 0
};

struct EngineUboStruct {
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 eyePos;
};

struct LightmapStruct {
	glm::vec3 lightColor = glm::vec3(3, 0.8, 0.4);
	float lightAttenuationRadius = 40.0f;
	glm::vec3 lightPosition = glm::vec3(1, 2, 1);
	float lightIntensity = 40.0f;
};

DeferredRenderer::DeferredRenderer() {
	CreateDeferredRendererInstanceObjects();
	CreateDeferredRendererStaticObjects();
}

DeferredRenderer::~DeferredRenderer() {
	auto core = EngineCore::GetInstance().GetGraphicsCore();
	core->DeleteUniformBufferBinding(globalUniformBufferBinding);
	core->DeleteUniformBuffer(globalUniformBufferObject);
	core->DeleteUniformBufferBinding(lightUniformBufferBinding);
	core->DeleteUniformBuffer(lightUniformBufferObject);

	core->DeleteFramebuffer(gbuffer);
	core->DeleteRenderTarget(gbufferRenderTargets);
	core->DeleteDepthTarget(gbufferDepthTarget);

	core->DeleteFramebuffer(litHdrFramebuffer);
	core->DeleteRenderTarget(litHdrRenderTarget);
	core->DeleteDepthTarget(litHdrDepthTarget);

	core->DeleteVertexArrayObject(planePostProcessVao);
	core->DeletePipeline(lightPipeline);
	core->DeletePipeline(tonemapPipeline);
}

bool DeferredRenderer::OnWindowResize(Events::BaseEvent* ev) {
	if (ev->GetEventType() == Events::EventType::WindowResize) {
		Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
		Resize(winResizeEvent->width, winResizeEvent->height);
	}

	return false;
}

void DeferredRenderer::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	gbuffer->Resize(width, height);
	litHdrFramebuffer->Resize(width, height);
}

// NOTE: Make these objects static
void DeferredRenderer::CreateDeferredRendererStaticObjects() {
	auto core = EngineCore::GetInstance().GetGraphicsCore();

	UniformBufferBinding::CreateInfo globalUniformBufferBindingCi{};
	globalUniformBufferBindingCi.binding = 0;
	globalUniformBufferBindingCi.shaderLocation = "EngineUbo";
	globalUniformBufferBindingCi.size = sizeof(EngineUboStruct);
	globalUniformBufferBindingCi.stages = ShaderStageBit::AllGraphics;
	globalUniformBufferBinding = core->CreateUniformBufferBinding(globalUniformBufferBindingCi);

	UniformBuffer::CreateInfo globalUniformBufferObjectCi{};
	globalUniformBufferObjectCi.binding = globalUniformBufferBinding;
	globalUniformBufferObjectCi.isDynamic = true;
	globalUniformBufferObjectCi.size = sizeof(EngineUboStruct);
	globalUniformBufferObject = core->CreateUniformBuffer(globalUniformBufferObjectCi);

	// ========= Light Stuff =========
	UniformBufferBinding::CreateInfo lightUniformBufferBindingCi{};
	lightUniformBufferBindingCi.binding = 1;
	lightUniformBufferBindingCi.shaderLocation = "LightUbo";
	lightUniformBufferBindingCi.size = sizeof(LightmapStruct);
	lightUniformBufferBindingCi.stages = ShaderStageBit::AllGraphics;
	lightUniformBufferBinding = core->CreateUniformBufferBinding(lightUniformBufferBindingCi);

	UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
	lightUniformBufferObjectCi.binding = lightUniformBufferBinding;
	lightUniformBufferObjectCi.isDynamic = true;
	lightUniformBufferObjectCi.size = sizeof(LightmapStruct);
	lightUniformBufferObject = core->CreateUniformBuffer(lightUniformBufferObjectCi);

	LightmapStruct lightmapStruct;
	lightUniformBufferObject->UpdateBuffer(&lightmapStruct);

	VertexBufferLayout vertexLightPositionLayout({
		{
			0,
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexPosition",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
	});

	VertexBuffer::CreateInfo vboCi{};
	vboCi.debugName = "Light Vertex Position Buffer";
	vboCi.content = lightPositions;
	vboCi.count = sizeof(lightPositions) / (sizeof(float) * 2);
	vboCi.size = sizeof(lightPositions);
	vboCi.layout = &vertexLightPositionLayout;
	VertexBuffer* vbo = core->CreateVertexBuffer(vboCi);

	IndexBuffer::CreateInfo iboCi{};
	iboCi.debugName = "Light Index Buffer";
	iboCi.content = lightIndices;
	iboCi.count = sizeof(lightIndices) / sizeof(lightIndices[0]);
	iboCi.size = sizeof(lightIndices);
	IndexBuffer* ibo = core->CreateIndexBuffer(iboCi);

	VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = "Light Vertex Array Object";
	vaoCi.vertexBufferCount = 1;
	vaoCi.vertexBuffers = &vbo;
	vaoCi.indexBuffer = ibo;
	planePostProcessVao = core->CreateVertexArrayObject(vaoCi);
}

void DeferredRenderer::CreateDeferredRendererInstanceObjects() {
	auto core = EngineCore::GetInstance().GetGraphicsCore();

	const uint32_t width = 800;
	const uint32_t height = 600;
	std::vector<RenderTarget::CreateInfo> gbufferImagesCreateInfo;
	gbufferImagesCreateInfo.reserve(4);
	gbufferImagesCreateInfo.emplace_back(ColorFormat::R16G16B16A16, width, height); // X Y Z
	gbufferImagesCreateInfo.emplace_back(ColorFormat::R8G8B8A8, width, height); // R  G  B matID
	gbufferImagesCreateInfo.emplace_back(ColorFormat::R16G16B16A16, width, height); // nX nY nZ
	gbufferImagesCreateInfo.emplace_back(ColorFormat::R8G8B8A8, width, height); // sR sG sB Roughness
	gbufferRenderTargets = core->CreateRenderTarget(gbufferImagesCreateInfo.data(), (uint32_t)gbufferImagesCreateInfo.size());

	std::vector<ColorFormat> colorFormats;
	colorFormats.reserve(gbufferImagesCreateInfo.size());
	for (size_t i = 0; i < gbufferImagesCreateInfo.size(); ++i) {
		colorFormats.emplace_back(gbufferImagesCreateInfo[i].format);
	}

	RenderPass::CreateInfo gbufferRenderPassCreateInfo{};
	gbufferRenderPassCreateInfo.width = width;
	gbufferRenderPassCreateInfo.height = height;
	gbufferRenderPassCreateInfo.colorFormats = colorFormats.data();
	gbufferRenderPassCreateInfo.colorFormatCount = static_cast<uint32_t>(colorFormats.size());
	gbufferRenderPassCreateInfo.depthFormat = DepthFormat::D24_STENCIL_8;
	gbufferRenderPass = core->CreateRenderPass(gbufferRenderPassCreateInfo);

	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(gbufferRenderPassCreateInfo.depthFormat, width, height, false, false);
	gbufferDepthTarget = core->CreateDepthTarget(gbufferDepthImageCreateInfo);

	Framebuffer::CreateInfo gbufferCreateInfo{};
	gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
	gbufferCreateInfo.renderPass = gbufferRenderPass;
	gbufferCreateInfo.renderTargetLists = &gbufferRenderTargets;
	gbufferCreateInfo.numRenderTargetLists = 1;
	gbufferCreateInfo.depthTarget = gbufferDepthTarget;
	gbuffer = core->CreateFramebuffer(gbufferCreateInfo);

	RenderTarget::CreateInfo litHdrImagesCreateInfo
		= { Grindstone::GraphicsAPI::ColorFormat::R32G32B32, width, height };
	litHdrRenderTarget = core->CreateRenderTarget(&litHdrImagesCreateInfo, 1);

	DepthTarget::CreateInfo litHdrDepthImageCreateInfo(DepthFormat::D24_STENCIL_8, width, height, false, false);
	litHdrDepthTarget = core->CreateDepthTarget(litHdrDepthImageCreateInfo);

	RenderPass::CreateInfo mainRenderPassCreateInfo{};
	mainRenderPassCreateInfo.width = width;
	mainRenderPassCreateInfo.height = height;
	mainRenderPassCreateInfo.colorFormats = &litHdrImagesCreateInfo.format;
	mainRenderPassCreateInfo.colorFormatCount = 1;
	mainRenderPassCreateInfo.depthFormat = litHdrDepthImageCreateInfo.format;
	mainRenderPass = core->CreateRenderPass(mainRenderPassCreateInfo);

	Framebuffer::CreateInfo litHdrFramebufferCreateInfo{};
	litHdrFramebufferCreateInfo.debugName = "Lit HDR Framebuffer";
	litHdrFramebufferCreateInfo.renderTargetLists = &litHdrRenderTarget;
	litHdrFramebufferCreateInfo.numRenderTargetLists = 1;
	litHdrFramebufferCreateInfo.depthTarget = litHdrDepthTarget;
	litHdrFramebufferCreateInfo.renderPass = mainRenderPass;
	litHdrFramebuffer = core->CreateFramebuffer(litHdrFramebufferCreateInfo);
	
	auto assetManager = EngineCore::GetInstance().assetManager;
	ShaderAsset* lightShaderAsset = assetManager->GetAsset<ShaderAsset>(Uuid("5537b925-96bc-4e1f-8e2a-d66d6dd9bed1"));
	if (lightShaderAsset == nullptr) {
		EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load point light shader.");
	}
	else {
		lightPipeline = lightShaderAsset->pipeline;
	}

	ShaderAsset* tonemapShaderAsset = assetManager->GetAsset<ShaderAsset>(Uuid("30e9223e-1753-4a7a-acac-8488c75bb1ef"));
	if (tonemapShaderAsset == nullptr) {
		EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load tonemap shader.");
	}
	else {
		tonemapPipeline = tonemapShaderAsset->pipeline;
	}
}

void DeferredRenderer::RenderLights(entt::registry& registry) {
	GRIND_PROFILE_FUNC();
	if (lightPipeline == nullptr) {
		return;
	}

	auto core = EngineCore::GetInstance().GetGraphicsCore();

	core->BindPipeline(lightPipeline);
	core->EnableDepthWrite(false);
	litHdrFramebuffer->BindWrite();

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.f };
	core->Clear(ClearMode::ColorAndDepth, clearColor, 1);
	core->SetImmediateBlending(BlendMode::Additive);
	gbuffer->BindTextures(2);

	auto view = registry.view<const TransformComponent, const PointLightComponent>();
	view.each([&](const TransformComponent& transformComponent, const PointLightComponent& pointLightComponent) {
		LightmapStruct lightmapStruct{
			pointLightComponent.color,
			pointLightComponent.attenuationRadius,
			transformComponent.position,
			pointLightComponent.intensity,
		};

		lightUniformBufferObject->UpdateBuffer(&lightmapStruct);
		lightUniformBufferObject->Bind();
		planePostProcessVao->Bind();
		core->DrawImmediateIndexed(GeometryType::Triangles, false, 0, 0, 6);
	});
}

void DeferredRenderer::PostProcess(GraphicsAPI::Framebuffer* outputFramebuffer) {
	GRIND_PROFILE_FUNC();
	if (tonemapPipeline == nullptr) {
		return;
	}

	auto core = EngineCore::GetInstance().GetGraphicsCore();

	core->BindPipeline(tonemapPipeline);
	core->EnableDepthWrite(true);
	if (outputFramebuffer == nullptr) {
		core->BindDefaultFramebufferWrite();
	}
	else {
		outputFramebuffer->BindWrite();
	}
	litHdrFramebuffer->BindRead();

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.f };
	core->Clear(ClearMode::ColorAndDepth, clearColor, 1);
	core->SetImmediateBlending(BlendMode::None);
	litHdrFramebuffer->BindTextures(1);
	planePostProcessVao->Bind();
	core->DrawImmediateIndexed(GeometryType::Triangles, false, 0, 0, 6);
}

void DeferredRenderer::Render(
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
	auto core = EngineCore::GetInstance().GetGraphicsCore();
	core->ResizeViewport(width, height);

	EngineUboStruct engineUboStruct{};
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.eyePos = eyePos;

	gbuffer->BindWrite();
	gbuffer->BindRead();

	float clearColor[4] = { 0.3f, 0.6f, 0.9f, 1.f };
	core->Clear(ClearMode::ColorAndDepth, clearColor, 1);

	globalUniformBufferObject->UpdateBuffer(&engineUboStruct);
	globalUniformBufferObject->Bind();

	core->EnableDepthWrite(true);
	core->SetImmediateBlending(BlendMode::None);
	EngineCore::GetInstance().assetRendererManager->RenderQueue("Opaque");

	RenderLights(registry);

	EngineCore::GetInstance().assetRendererManager->RenderQueue("Unlit");

	core->EnableDepthWrite(false);
	core->CopyDepthBufferFromReadToWrite(width, height, width, height);
	core->SetImmediateBlending(BlendMode::AdditiveAlpha);
	EngineCore::GetInstance().assetRendererManager->RenderQueue("Transparent");

	PostProcess(outputFramebuffer);
}
