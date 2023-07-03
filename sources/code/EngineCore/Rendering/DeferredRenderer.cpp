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
#include <Common/Window/WindowManager.hpp>
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
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	CreateCommandBuffers();
	CreateVertexAndIndexBuffersAndLayouts();
	CreateGbufferFramebuffer();
	CreateLitHDRFramebuffer();
	CreateUniformBuffers();
	CreateDescriptorSetLayouts();
	CreateDescriptorSets();
	CreatePipelines();
}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->DeleteUniformBuffer(globalUniformBufferObject);
	graphicsCore->DeleteUniformBuffer(lightUniformBufferObject);

	graphicsCore->DeleteFramebuffer(gbuffer);
	for (size_t i = 0; i < gbufferRenderTargets.size(); ++i) {
		graphicsCore->DeleteRenderTarget(gbufferRenderTargets[i]);
	}
	graphicsCore->DeleteDepthTarget(gbufferDepthTarget);

	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		graphicsCore->DeleteCommandBuffer(commandBuffers[i]);
	}

	graphicsCore->DeleteFramebuffer(litHdrFramebuffer);
	graphicsCore->DeleteRenderTarget(litHdrRenderTarget);
	graphicsCore->DeleteDepthTarget(litHdrDepthTarget);

	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);
	graphicsCore->DeletePipeline(pointLightPipeline);
	graphicsCore->DeletePipeline(tonemapPipeline);
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
	return;
	gbuffer->Resize(width, height);
	litHdrFramebuffer->Resize(width, height);
}

void DeferredRenderer::CreateCommandBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto wgb = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

	GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();
	commandBuffers.resize(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; ++i) {
		commandBuffers[i] = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);
	}
}

void DeferredRenderer::CreateUniformBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	UniformBuffer::CreateInfo globalUniformBufferObjectCi{};
	globalUniformBufferObjectCi.debugName = "EngineUbo";
	globalUniformBufferObjectCi.isDynamic = true;
	globalUniformBufferObjectCi.size = sizeof(EngineUboStruct);
	globalUniformBufferObject = graphicsCore->CreateUniformBuffer(globalUniformBufferObjectCi);

	UniformBuffer::CreateInfo lightUniformBufferObjectCi{};
	lightUniformBufferObjectCi.debugName = "LightUbo";
	lightUniformBufferObjectCi.isDynamic = true;
	lightUniformBufferObjectCi.size = sizeof(LightmapStruct);
	lightUniformBufferObject = graphicsCore->CreateUniformBuffer(lightUniformBufferObjectCi);

	LightmapStruct lightmapStruct;
	lightUniformBufferObject->UpdateBuffer(&lightmapStruct);
}

void DeferredRenderer::CreateDescriptorSetLayouts() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSetLayout::Binding engineUboBinding{};
	engineUboBinding.bindingId = 0;
	engineUboBinding.count = 1;
	engineUboBinding.type = BindingType::UniformBuffer;
	engineUboBinding.stages = ShaderStageBit::Vertex | ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding lightUboBinding{};
	lightUboBinding.bindingId = 1;
	lightUboBinding.count = 1;
	lightUboBinding.type = BindingType::UniformBuffer;
	lightUboBinding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding litHdrRenderTargetBinding{};
	litHdrRenderTargetBinding.bindingId = 1;
	litHdrRenderTargetBinding.count = 1;
	litHdrRenderTargetBinding.type = BindingType::Texture;
	litHdrRenderTargetBinding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer0Binding{};
	gbuffer0Binding.bindingId = 2;
	gbuffer0Binding.count = 1;
	gbuffer0Binding.type = BindingType::Texture;
	gbuffer0Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer1Binding{};
	gbuffer1Binding.bindingId = 3;
	gbuffer1Binding.count = 1;
	gbuffer1Binding.type = BindingType::Texture;
	gbuffer1Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer2Binding{};
	gbuffer2Binding.bindingId = 4;
	gbuffer2Binding.count = 1;
	gbuffer2Binding.type = BindingType::Texture;
	gbuffer2Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer3Binding{};
	gbuffer3Binding.bindingId = 5;
	gbuffer3Binding.count = 1;
	gbuffer3Binding.type = BindingType::Texture;
	gbuffer3Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo engineDescriptorSetLayoutCreateInfo{};
	engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
	engineDescriptorSetLayoutCreateInfo.bindings = &engineUboBinding;
	engineDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 2> tonemapDescriptorSetLayoutBindings{};
	tonemapDescriptorSetLayoutBindings[0] = engineUboBinding;
	tonemapDescriptorSetLayoutBindings[1] = litHdrRenderTargetBinding;

	DescriptorSetLayout::CreateInfo tonemapDescriptorSetLayoutCreateInfo{};
	tonemapDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetLayoutBindings.size());
	tonemapDescriptorSetLayoutCreateInfo.bindings = tonemapDescriptorSetLayoutBindings.data();
	tonemapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(tonemapDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 6> lightingDescriptorSetLayoutBindings{};
	lightingDescriptorSetLayoutBindings[0] = engineUboBinding;
	lightingDescriptorSetLayoutBindings[1] = lightUboBinding;
	lightingDescriptorSetLayoutBindings[2] = gbuffer0Binding;
	lightingDescriptorSetLayoutBindings[3] = gbuffer1Binding;
	lightingDescriptorSetLayoutBindings[4] = gbuffer2Binding;
	lightingDescriptorSetLayoutBindings[5] = gbuffer3Binding;

	DescriptorSetLayout::CreateInfo lightingDescriptorSetLayoutCreateInfo{};
	lightingDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightingDescriptorSetLayoutBindings.size());
	lightingDescriptorSetLayoutCreateInfo.bindings = lightingDescriptorSetLayoutBindings.data();
	lightingDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(lightingDescriptorSetLayoutCreateInfo);
}

void DeferredRenderer::CreateDescriptorSets() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSet::Binding engineUboBinding{};
	engineUboBinding.bindingIndex = 0;
	engineUboBinding.count = 1;
	engineUboBinding.bindingType = BindingType::UniformBuffer;
	engineUboBinding.itemPtr = globalUniformBufferObject;

	DescriptorSet::Binding lightUboBinding{};
	lightUboBinding.bindingIndex = 1;
	lightUboBinding.count = 1;
	lightUboBinding.bindingType = BindingType::UniformBuffer;
	lightUboBinding.itemPtr = lightUniformBufferObject;

	DescriptorSet::Binding litHdrRenderTargetBinding{};
	litHdrRenderTargetBinding.bindingIndex = 1;
	litHdrRenderTargetBinding.count = 1;
	litHdrRenderTargetBinding.bindingType = BindingType::RenderTexture;
	litHdrRenderTargetBinding.itemPtr = litHdrRenderTarget;

	DescriptorSet::Binding gbuffer0Binding{};
	gbuffer0Binding.bindingIndex = 2;
	gbuffer0Binding.count = 1;
	gbuffer0Binding.bindingType = BindingType::RenderTexture;
	gbuffer0Binding.itemPtr = gbufferRenderTargets[0];

	DescriptorSet::Binding gbuffer1Binding{};
	gbuffer1Binding.bindingIndex = 3;
	gbuffer1Binding.count = 1;
	gbuffer1Binding.bindingType = BindingType::RenderTexture;
	gbuffer1Binding.itemPtr = gbufferRenderTargets[1];

	DescriptorSet::Binding gbuffer2Binding{};
	gbuffer2Binding.bindingIndex = 4;
	gbuffer2Binding.count = 1;
	gbuffer2Binding.bindingType = BindingType::RenderTexture;
	gbuffer2Binding.itemPtr = gbufferRenderTargets[2];

	DescriptorSet::Binding gbuffer3Binding{};
	gbuffer3Binding.bindingIndex = 5;
	gbuffer3Binding.count = 1;
	gbuffer3Binding.bindingType = BindingType::RenderTexture;
	gbuffer3Binding.itemPtr = gbufferRenderTargets[3];

	DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
	engineDescriptorSetCreateInfo.layout = engineDescriptorSetLayout;
	engineDescriptorSetCreateInfo.bindingCount = 1;
	engineDescriptorSetCreateInfo.bindings = &engineUboBinding;
	engineDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 2> tonemapDescriptorSetBindings{};
	tonemapDescriptorSetBindings[0] = engineUboBinding;
	tonemapDescriptorSetBindings[1] = litHdrRenderTargetBinding;

	DescriptorSet::CreateInfo tonemapDescriptorSetCreateInfo{};
	tonemapDescriptorSetCreateInfo.layout = tonemapDescriptorSetLayout;
	tonemapDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetBindings.size());
	tonemapDescriptorSetCreateInfo.bindings = tonemapDescriptorSetBindings.data();
	tonemapDescriptorSet = graphicsCore->CreateDescriptorSet(tonemapDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 6> lightingDescriptorSetBindings{};
	lightingDescriptorSetBindings[0] = engineUboBinding;
	lightingDescriptorSetBindings[1] = lightUboBinding;
	lightingDescriptorSetBindings[2] = gbuffer0Binding;
	lightingDescriptorSetBindings[3] = gbuffer1Binding;
	lightingDescriptorSetBindings[4] = gbuffer2Binding;
	lightingDescriptorSetBindings[5] = gbuffer3Binding;

	DescriptorSet::CreateInfo lightingDescriptorSetCreateInfo{};
	lightingDescriptorSetCreateInfo.layout = lightingDescriptorSetLayout;
	lightingDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightingDescriptorSetBindings.size());
	lightingDescriptorSetCreateInfo.bindings = lightingDescriptorSetBindings.data();
	lightingDescriptorSet = graphicsCore->CreateDescriptorSet(lightingDescriptorSetCreateInfo);
}

void DeferredRenderer::CreateVertexAndIndexBuffersAndLayouts() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	vertexLightPositionLayout = {
		{
			0,
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexPosition",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
	};

	VertexBuffer::CreateInfo vboCi{};
	vboCi.debugName = "Light Vertex Position Buffer";
	vboCi.content = lightPositions;
	vboCi.count = sizeof(lightPositions) / (sizeof(float) * 2);
	vboCi.size = sizeof(lightPositions);
	vboCi.layout = &vertexLightPositionLayout;
	vertexBuffer = graphicsCore->CreateVertexBuffer(vboCi);

	IndexBuffer::CreateInfo iboCi{};
	iboCi.debugName = "Light Index Buffer";
	iboCi.content = lightIndices;
	iboCi.count = sizeof(lightIndices) / sizeof(lightIndices[0]);
	iboCi.size = sizeof(lightIndices);
	indexBuffer = graphicsCore->CreateIndexBuffer(iboCi);

	VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = "Light Vertex Array Object";
	vaoCi.vertexBufferCount = 1;
	vaoCi.vertexBuffers = &vertexBuffer;
	vaoCi.indexBuffer = indexBuffer;
	planePostProcessVao = graphicsCore->CreateVertexArrayObject(vaoCi);
}

void DeferredRenderer::CreateGbufferFramebuffer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	std::vector<ColorFormat> gbufferColorFormats;
	gbufferColorFormats.reserve(4);
	gbufferColorFormats.emplace_back(ColorFormat::R16G16B16A16); // X Y Z
	gbufferColorFormats.emplace_back(ColorFormat::R8G8B8A8); // R  G  B matID
	gbufferColorFormats.emplace_back(ColorFormat::R16G16B16A16); // nX nY nZ
	gbufferColorFormats.emplace_back(ColorFormat::R8G8B8A8); // sR sG sB Roughness

	gbufferRenderTargets.reserve(gbufferColorFormats.size());
	for (size_t i = 0; i < gbufferColorFormats.size(); ++i) {
		RenderTarget::CreateInfo gbufferRtCreateInfo{ gbufferColorFormats[i], width, height, true };
		gbufferRenderTargets.emplace_back(graphicsCore->CreateRenderTarget(gbufferRtCreateInfo));
	}

	RenderPass::CreateInfo gbufferRenderPassCreateInfo{};
	gbufferRenderPassCreateInfo.width = width;
	gbufferRenderPassCreateInfo.height = height;
	gbufferRenderPassCreateInfo.colorFormats = gbufferColorFormats.data();
	gbufferRenderPassCreateInfo.colorFormatCount = static_cast<uint32_t>(gbufferColorFormats.size());
	gbufferRenderPassCreateInfo.depthFormat = DepthFormat::D24_STENCIL_8;
	gbufferRenderPass = graphicsCore->CreateRenderPass(gbufferRenderPassCreateInfo);

	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(gbufferRenderPassCreateInfo.depthFormat, width, height, false, false);
	gbufferDepthTarget = graphicsCore->CreateDepthTarget(gbufferDepthImageCreateInfo);

	Framebuffer::CreateInfo gbufferCreateInfo{};
	gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
	gbufferCreateInfo.renderPass = gbufferRenderPass;
	gbufferCreateInfo.renderTargetLists = gbufferRenderTargets.data();
	gbufferCreateInfo.numRenderTargetLists = static_cast<uint32_t>(gbufferRenderTargets.size());
	gbufferCreateInfo.depthTarget = gbufferDepthTarget;
	gbuffer = graphicsCore->CreateFramebuffer(gbufferCreateInfo);
}

void DeferredRenderer::CreateLitHDRFramebuffer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	RenderTarget::CreateInfo litHdrImagesCreateInfo = { Grindstone::GraphicsAPI::ColorFormat::R16G16B16A16, width, height, true };
	litHdrRenderTarget = graphicsCore->CreateRenderTarget(litHdrImagesCreateInfo);

	DepthTarget::CreateInfo litHdrDepthImageCreateInfo(DepthFormat::D24_STENCIL_8, width, height, false, false);
	litHdrDepthTarget = graphicsCore->CreateDepthTarget(litHdrDepthImageCreateInfo);

	RenderPass::CreateInfo mainRenderPassCreateInfo{};
	mainRenderPassCreateInfo.width = width;
	mainRenderPassCreateInfo.height = height;
	mainRenderPassCreateInfo.colorFormats = &litHdrImagesCreateInfo.format;
	mainRenderPassCreateInfo.colorFormatCount = 1;
	mainRenderPassCreateInfo.depthFormat = litHdrDepthImageCreateInfo.format;
	mainRenderPass = graphicsCore->CreateRenderPass(mainRenderPassCreateInfo);

	Framebuffer::CreateInfo litHdrFramebufferCreateInfo{};
	litHdrFramebufferCreateInfo.debugName = "Lit HDR Framebuffer";
	litHdrFramebufferCreateInfo.renderTargetLists = &litHdrRenderTarget;
	litHdrFramebufferCreateInfo.numRenderTargetLists = 1;
	litHdrFramebufferCreateInfo.depthTarget = litHdrDepthTarget;
	litHdrFramebufferCreateInfo.renderPass = mainRenderPass;
	litHdrFramebuffer = graphicsCore->CreateFramebuffer(litHdrFramebufferCreateInfo);
}

void DeferredRenderer::CreatePipelines() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	Pipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.primitiveType = GeometryType::Triangles;
	pipelineCreateInfo.cullMode = CullMode::None;
	pipelineCreateInfo.width = 800;
	pipelineCreateInfo.height = 600;
	pipelineCreateInfo.scissorX = 0;
	pipelineCreateInfo.scissorY = 0;
	pipelineCreateInfo.scissorW = 800;
	pipelineCreateInfo.scissorH = 600;
	pipelineCreateInfo.vertexBindings = &vertexLightPositionLayout;
	pipelineCreateInfo.vertexBindingsCount = 1;

	std::vector<ShaderStageCreateInfo> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	auto assetManager = EngineCore::GetInstance().assetManager;
	uint8_t shaderBits = static_cast<uint8_t>(ShaderStageBit::Vertex | ShaderStageBit::Fragment);

	{
		if (!assetManager->LoadShaderSet(
			Uuid("5537b925-96bc-4e1f-8e2a-d66d6dd9bed1"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load point light shaders.");
			return;
		}

		pipelineCreateInfo.shaderName = "Point Light Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = &lightingDescriptorSetLayout;
		pipelineCreateInfo.descriptorSetLayoutCount = 1;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::Additive;
		pipelineCreateInfo.renderPass = mainRenderPass;
		pointLightPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(
			Uuid("30e9223e-1753-4a7a-acac-8488c75bb1ef"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load tonemapping shaders.");
			return;
		}

		auto wgb = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

		pipelineCreateInfo.shaderName = "Tonemapping Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = &tonemapDescriptorSetLayout;
		pipelineCreateInfo.descriptorSetLayoutCount = 1;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = wgb->GetRenderPass();
		tonemapPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}
}

void DeferredRenderer::RenderLightsImmediate(entt::registry& registry) {
	GRIND_PROFILE_FUNC();
	if (pointLightPipeline == nullptr) {
		return;
	}

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	graphicsCore->BindPipeline(pointLightPipeline);
	graphicsCore->EnableDepthWrite(false);
	litHdrFramebuffer->BindWrite();

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.f };
	graphicsCore->Clear(ClearMode::ColorAndDepth, clearColor, 1);
	graphicsCore->SetImmediateBlending(BlendMode::Additive);
	gbuffer->BindTextures(2);
	lightUniformBufferObject->Bind();
	planePostProcessVao->Bind();

	auto view = registry.view<const TransformComponent, const PointLightComponent>();
	view.each([&](const TransformComponent& transformComponent, const PointLightComponent& pointLightComponent) {
		LightmapStruct lightmapStruct{
			pointLightComponent.color,
			pointLightComponent.attenuationRadius,
			transformComponent.position,
			pointLightComponent.intensity,
		};

		lightUniformBufferObject->UpdateBuffer(&lightmapStruct);
		graphicsCore->DrawImmediateIndexed(GeometryType::Triangles, false, 0, 0, 6);
		});
}

void DeferredRenderer::RenderLightsCommandBuffer(GraphicsAPI::CommandBuffer* currentCommandBuffer, entt::registry& registry) {
	GRIND_PROFILE_FUNC();
	if (pointLightPipeline == nullptr) {
		return;
	}

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	ClearColorValue clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;
	currentCommandBuffer->BindRenderPass(mainRenderPass, litHdrFramebuffer, 800, 600, &clearColor, 1, clearDepthStencil);

	currentCommandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	currentCommandBuffer->BindIndexBuffer(indexBuffer);

	{
		// Point Lights
		currentCommandBuffer->BindPipeline(pointLightPipeline);
		currentCommandBuffer->BindDescriptorSet(pointLightPipeline, &lightingDescriptorSet, 1);

		auto view = registry.view<const TransformComponent, const PointLightComponent>();
		view.each([&](const TransformComponent& transformComponent, const PointLightComponent& pointLightComponent) {
			LightmapStruct lightmapStruct{
				pointLightComponent.color,
				pointLightComponent.attenuationRadius,
				transformComponent.position,
				pointLightComponent.intensity,
			};

			lightUniformBufferObject->UpdateBuffer(&lightmapStruct);
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}

	currentCommandBuffer->UnbindRenderPass();
}

void DeferredRenderer::PostProcessImmediate(GraphicsAPI::Framebuffer* outputFramebuffer) {

}

void DeferredRenderer::PostProcessCommandBuffer(GraphicsAPI::RenderPass* renderPass, GraphicsAPI::Framebuffer* framebuffer, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	GRIND_PROFILE_FUNC();
	
	ClearColorValue clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = false;
	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, 800, 600, &clearColor, 1, clearDepthStencil);
	
	currentCommandBuffer->BindDescriptorSet(tonemapPipeline, &tonemapDescriptorSet, 1);
	currentCommandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	currentCommandBuffer->BindIndexBuffer(indexBuffer);

	{
		// Tonemapping
		currentCommandBuffer->BindPipeline(tonemapPipeline);
		currentCommandBuffer->DrawIndices(0, 6, 1, 0);
	}

	currentCommandBuffer->UnbindRenderPass();
}

void DeferredRenderer::Render(
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
	EngineUboStruct engineUboStruct{};
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.eyePos = eyePos;
	globalUniformBufferObject->UpdateBuffer(&engineUboStruct);

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	if (graphicsCore->ShouldUseImmediateMode()) {
		RenderImmediate(
			registry,
			outputFramebuffer
		);
	}
	else {
		RenderCommandBuffer(
			registry,
			outputFramebuffer
		);
	}
}

void DeferredRenderer::RenderCommandBuffer(
	entt::registry& registry,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto assetManager = EngineCore::GetInstance().assetRendererManager;
	auto wgb = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	wgb->AcquireNextImage();

	GraphicsAPI::RenderPass* targetRenderPass = nullptr;
	GraphicsAPI::Framebuffer* targetFramebuffer = outputFramebuffer;
	if (outputFramebuffer == nullptr) {
		targetRenderPass = wgb->GetRenderPass();
		targetFramebuffer = wgb->GetCurrentFramebuffer();
	}

	auto currentCommandBuffer = commandBuffers[wgb->GetCurrentImageIndex()];
	currentCommandBuffer->BeginCommandBuffer();

	ClearColorValue clearColors[] = {
		ClearColorValue{0.3f, 0.6f, 0.9f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f}
	};

	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;
	assetManager->SetEngineDescriptorSet(engineDescriptorSet);
	currentCommandBuffer->BindRenderPass(gbufferRenderPass, gbuffer, 800, 600, clearColors, 5, clearDepthStencil);
	// assetManager->RenderQueue(currentCommandBuffer, "Opaque");
	currentCommandBuffer->UnbindRenderPass();
	RenderLightsCommandBuffer(currentCommandBuffer, registry);
	// assetManager->RenderQueue("Unlit");
	// assetManager->RenderQueue("Transparent");

	PostProcessCommandBuffer(targetRenderPass, targetFramebuffer, currentCommandBuffer);

	currentCommandBuffer->EndCommandBuffer();
	wgb->SubmitCommandBuffer(currentCommandBuffer);
	wgb->PresentSwapchain();
}

void DeferredRenderer::RenderImmediate(
	entt::registry& registry,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto assetManager = EngineCore::GetInstance().assetRendererManager;
	graphicsCore->ResizeViewport(width, height);

	gbuffer->BindWrite();
	gbuffer->BindRead();

	float clearColor[4] = { 0.3f, 0.6f, 0.9f, 1.f };
	graphicsCore->Clear(ClearMode::ColorAndDepth, clearColor, 1);

	globalUniformBufferObject->Bind();
	assetManager->SetEngineDescriptorSet(engineDescriptorSet);

	graphicsCore->EnableDepthWrite(true);
	graphicsCore->SetImmediateBlending(BlendMode::None);
	EngineCore::GetInstance().assetRendererManager->RenderQueueImmediate("Opaque");

	RenderLightsImmediate(registry);

	EngineCore::GetInstance().assetRendererManager->RenderQueueImmediate("Unlit");

	graphicsCore->EnableDepthWrite(false);
	graphicsCore->CopyDepthBufferFromReadToWrite(width, height, width, height);
	graphicsCore->SetImmediateBlending(BlendMode::AdditiveAlpha);
	EngineCore::GetInstance().assetRendererManager->RenderQueueImmediate("Transparent");

	PostProcessImmediate(outputFramebuffer);
}
