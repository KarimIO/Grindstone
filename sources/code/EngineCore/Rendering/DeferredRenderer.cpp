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
#include "EngineCore/CoreComponents/Lights/SpotLightComponent.hpp"
#include "EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp"
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

DeferredRenderer::DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto wgb = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();
	deferredRendererImageSets.resize(maxFramesInFlight);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.width = 1024;
	renderPassCreateInfo.height = 1024;
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	shadowMapRenderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	CreateCommandBuffers();
	CreateVertexAndIndexBuffersAndLayouts();
	CreateGbufferFramebuffer();
	CreateLitHDRFramebuffer();
	CreateUniformBuffers();
	CreateDescriptorSetLayouts();
	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		CreateDescriptorSets(deferredRendererImageSets[i]);
	}

	CreatePipelines();
}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	graphicsCore->DeletePipeline(pointLightPipeline);
	graphicsCore->DeletePipeline(tonemapPipeline);

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		graphicsCore->DeleteUniformBuffer(imageSet.globalUniformBufferObject);

		graphicsCore->DeleteFramebuffer(imageSet.gbuffer);
		for (size_t i = 0; i < imageSet.gbufferRenderTargets.size(); ++i) {
			graphicsCore->DeleteRenderTarget(imageSet.gbufferRenderTargets[i]);
		}
		graphicsCore->DeleteDepthTarget(imageSet.gbufferDepthTarget);
		graphicsCore->DeleteFramebuffer(imageSet.litHdrFramebuffer);
		graphicsCore->DeleteRenderTarget(imageSet.litHdrRenderTarget);
		graphicsCore->DeleteDepthTarget(imageSet.litHdrDepthTarget);
		graphicsCore->DeleteCommandBuffer(imageSet.commandBuffer);

		graphicsCore->DeleteDescriptorSet(imageSet.engineDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.tonemapDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.lightingDescriptorSet);
	}

	graphicsCore->DeleteDescriptorSetLayout(engineDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(tonemapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingDescriptorSetLayout);

	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);
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
	// gbuffer->Resize(width, height);
	// litHdrFramebuffer->Resize(width, height);
}

void DeferredRenderer::CreateCommandBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::CommandBuffer::CreateInfo commandBufferCreateInfo{};

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		deferredRendererImageSets[i].commandBuffer = graphicsCore->CreateCommandBuffer(commandBufferCreateInfo);
	}
}

void DeferredRenderer::CreateUniformBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		UniformBuffer::CreateInfo globalUniformBufferObjectCi{};
		globalUniformBufferObjectCi.debugName = "EngineUbo";
		globalUniformBufferObjectCi.isDynamic = true;
		globalUniformBufferObjectCi.size = sizeof(EngineUboStruct);
		imageSet.globalUniformBufferObject = graphicsCore->CreateUniformBuffer(globalUniformBufferObjectCi);
	}
}

void DeferredRenderer::CreateDescriptorSetLayouts() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSetLayout::Binding engineUboBinding{};
	engineUboBinding.bindingId = 0;
	engineUboBinding.count = 1;
	engineUboBinding.type = BindingType::UniformBuffer;
	engineUboBinding.stages = ShaderStageBit::Vertex | ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding litHdrRenderTargetBinding{};
	litHdrRenderTargetBinding.bindingId = 1;
	litHdrRenderTargetBinding.count = 1;
	litHdrRenderTargetBinding.type = BindingType::Texture;
	litHdrRenderTargetBinding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer0Binding{};
	gbuffer0Binding.bindingId = 1;
	gbuffer0Binding.count = 1;
	gbuffer0Binding.type = BindingType::Texture;
	gbuffer0Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer1Binding{};
	gbuffer1Binding.bindingId = 2;
	gbuffer1Binding.count = 1;
	gbuffer1Binding.type = BindingType::Texture;
	gbuffer1Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer2Binding{};
	gbuffer2Binding.bindingId = 3;
	gbuffer2Binding.count = 1;
	gbuffer2Binding.type = BindingType::Texture;
	gbuffer2Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer3Binding{};
	gbuffer3Binding.bindingId = 4;
	gbuffer3Binding.count = 1;
	gbuffer3Binding.type = BindingType::Texture;
	gbuffer3Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo engineDescriptorSetLayoutCreateInfo{};
	engineDescriptorSetLayoutCreateInfo.debugName = "Engine UBO Set Layout";
	engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
	engineDescriptorSetLayoutCreateInfo.bindings = &engineUboBinding;
	engineDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 2> tonemapDescriptorSetLayoutBindings{};
	tonemapDescriptorSetLayoutBindings[0] = engineUboBinding;
	tonemapDescriptorSetLayoutBindings[1] = litHdrRenderTargetBinding;

	DescriptorSetLayout::CreateInfo tonemapDescriptorSetLayoutCreateInfo{};
	tonemapDescriptorSetLayoutCreateInfo.debugName = "Tonemap Descriptor Set Layout";
	tonemapDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetLayoutBindings.size());
	tonemapDescriptorSetLayoutCreateInfo.bindings = tonemapDescriptorSetLayoutBindings.data();
	tonemapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(tonemapDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 5> lightingDescriptorSetLayoutBindings{};
	lightingDescriptorSetLayoutBindings[0] = engineUboBinding;
	lightingDescriptorSetLayoutBindings[1] = gbuffer0Binding;
	lightingDescriptorSetLayoutBindings[2] = gbuffer1Binding;
	lightingDescriptorSetLayoutBindings[3] = gbuffer2Binding;
	lightingDescriptorSetLayoutBindings[4] = gbuffer3Binding;

	DescriptorSetLayout::CreateInfo lightingDescriptorSetLayoutCreateInfo{};
	lightingDescriptorSetLayoutCreateInfo.debugName = "Pointlight Descriptor Set Layout";
	lightingDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightingDescriptorSetLayoutBindings.size());
	lightingDescriptorSetLayoutCreateInfo.bindings = lightingDescriptorSetLayoutBindings.data();
	lightingDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(lightingDescriptorSetLayoutCreateInfo);

	DescriptorSetLayout::Binding lightUboBinding{};
	lightUboBinding.bindingId = 0;
	lightUboBinding.count = 1;
	lightUboBinding.type = BindingType::UniformBuffer;
	lightUboBinding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo lightingUBODescriptorSetLayoutCreateInfo{};
	lightingUBODescriptorSetLayoutCreateInfo.debugName = "Pointlight UBO Descriptor Set Layout";
	lightingUBODescriptorSetLayoutCreateInfo.bindingCount = 1;
	lightingUBODescriptorSetLayoutCreateInfo.bindings = &lightUboBinding;
	lightingUBODescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(lightingUBODescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 2> shadowMappedLightBindings{};
	shadowMappedLightBindings[0].bindingId = 0;
	shadowMappedLightBindings[0].count = 1;
	shadowMappedLightBindings[0].type = BindingType::UniformBuffer;
	shadowMappedLightBindings[0].stages = ShaderStageBit::Fragment;

	shadowMappedLightBindings[1].bindingId = 1;
	shadowMappedLightBindings[1].count = 1;
	shadowMappedLightBindings[1].type = BindingType::DepthTexture;
	shadowMappedLightBindings[1].stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo shadowMappedLightDescriptorSetLayoutCreateInfo{};
	shadowMappedLightDescriptorSetLayoutCreateInfo.debugName = "Shadowmapped Light Descriptor Set Layout";
	shadowMappedLightDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(shadowMappedLightBindings.size());
	shadowMappedLightDescriptorSetLayoutCreateInfo.bindings = shadowMappedLightBindings.data();
	shadowMappedLightDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(shadowMappedLightDescriptorSetLayoutCreateInfo);

	DescriptorSetLayout::Binding shadowMapMatrixBinding{};
	shadowMapMatrixBinding.bindingId = 0;
	shadowMapMatrixBinding.count = 1;
	shadowMapMatrixBinding.type = BindingType::UniformBuffer;
	shadowMapMatrixBinding.stages = ShaderStageBit::Vertex;

	DescriptorSetLayout::CreateInfo shadowMapDescriptorSetLayoutCreateInfo{};
	shadowMapDescriptorSetLayoutCreateInfo.debugName = "Shadow Map Descriptor Set Layout";
	shadowMapDescriptorSetLayoutCreateInfo.bindingCount = 1;
	shadowMapDescriptorSetLayoutCreateInfo.bindings = &shadowMapMatrixBinding;
	shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(shadowMapDescriptorSetLayoutCreateInfo);
}

void DeferredRenderer::CreateDescriptorSets(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSet::Binding engineUboBinding{};
	engineUboBinding.bindingIndex = 0;
	engineUboBinding.count = 1;
	engineUboBinding.bindingType = BindingType::UniformBuffer;
	engineUboBinding.itemPtr = imageSet.globalUniformBufferObject;

	DescriptorSet::Binding litHdrRenderTargetBinding{};
	litHdrRenderTargetBinding.bindingIndex = 1;
	litHdrRenderTargetBinding.count = 1;
	litHdrRenderTargetBinding.bindingType = BindingType::RenderTexture;
	litHdrRenderTargetBinding.itemPtr = imageSet.litHdrRenderTarget;

	DescriptorSet::Binding gbuffer0Binding{};
	gbuffer0Binding.bindingIndex = 1;
	gbuffer0Binding.count = 1;
	gbuffer0Binding.bindingType = BindingType::RenderTexture;
	gbuffer0Binding.itemPtr = imageSet.gbufferRenderTargets[0];

	DescriptorSet::Binding gbuffer1Binding{};
	gbuffer1Binding.bindingIndex = 2;
	gbuffer1Binding.count = 1;
	gbuffer1Binding.bindingType = BindingType::RenderTexture;
	gbuffer1Binding.itemPtr = imageSet.gbufferRenderTargets[1];

	DescriptorSet::Binding gbuffer2Binding{};
	gbuffer2Binding.bindingIndex = 3;
	gbuffer2Binding.count = 1;
	gbuffer2Binding.bindingType = BindingType::RenderTexture;
	gbuffer2Binding.itemPtr = imageSet.gbufferRenderTargets[2];

	DescriptorSet::Binding gbuffer3Binding{};
	gbuffer3Binding.bindingIndex = 4;
	gbuffer3Binding.count = 1;
	gbuffer3Binding.bindingType = BindingType::RenderTexture;
	gbuffer3Binding.itemPtr = imageSet.gbufferRenderTargets[3];

	DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
	engineDescriptorSetCreateInfo.debugName = "Engine UBO Descriptor Set";
	engineDescriptorSetCreateInfo.layout = engineDescriptorSetLayout;
	engineDescriptorSetCreateInfo.bindingCount = 1;
	engineDescriptorSetCreateInfo.bindings = &engineUboBinding;
	imageSet.engineDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 2> tonemapDescriptorSetBindings{};
	tonemapDescriptorSetBindings[0] = engineUboBinding;
	tonemapDescriptorSetBindings[1] = litHdrRenderTargetBinding;

	DescriptorSet::CreateInfo tonemapDescriptorSetCreateInfo{};
	tonemapDescriptorSetCreateInfo.debugName = "Tonemap Descriptor Set";
	tonemapDescriptorSetCreateInfo.layout = tonemapDescriptorSetLayout;
	tonemapDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetBindings.size());
	tonemapDescriptorSetCreateInfo.bindings = tonemapDescriptorSetBindings.data();
	imageSet.tonemapDescriptorSet = graphicsCore->CreateDescriptorSet(tonemapDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 5> lightingDescriptorSetBindings{};
	lightingDescriptorSetBindings[0] = engineUboBinding;
	lightingDescriptorSetBindings[1] = gbuffer0Binding;
	lightingDescriptorSetBindings[2] = gbuffer1Binding;
	lightingDescriptorSetBindings[3] = gbuffer2Binding;
	lightingDescriptorSetBindings[4] = gbuffer3Binding;

	DescriptorSet::CreateInfo lightingDescriptorSetCreateInfo{};
	lightingDescriptorSetCreateInfo.debugName = "Point Light Descriptor Set";
	lightingDescriptorSetCreateInfo.layout = lightingDescriptorSetLayout;
	lightingDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightingDescriptorSetBindings.size());
	lightingDescriptorSetCreateInfo.bindings = lightingDescriptorSetBindings.data();
	imageSet.lightingDescriptorSet = graphicsCore->CreateDescriptorSet(lightingDescriptorSetCreateInfo);
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

	const int gbufferColorCount = 4;
	std::array<ColorFormat, gbufferColorCount> gbufferColorFormats{};
	gbufferColorFormats[0] = ColorFormat::R16G16B16A16; // X Y Z
	gbufferColorFormats[1] = ColorFormat::R8G8B8A8; // R  G  B matID
	gbufferColorFormats[2] = ColorFormat::R16G16B16A16; // nX nY nZ
	gbufferColorFormats[3] = ColorFormat::R8G8B8A8; // sR sG sB Roughness

	std::array<const char*, gbufferColorCount> gbufferColorAttachmentNames{};
	gbufferColorAttachmentNames[0] = "GBuffer Position Image";
	gbufferColorAttachmentNames[1] = "GBuffer Albedo Image";
	gbufferColorAttachmentNames[2] = "GBuffer Normal Image";
	gbufferColorAttachmentNames[3] = "GBuffer Speclar + Roughness Image";

	RenderPass::CreateInfo gbufferRenderPassCreateInfo{};
	gbufferRenderPassCreateInfo.width = width;
	gbufferRenderPassCreateInfo.height = height;
	gbufferRenderPassCreateInfo.colorFormats = gbufferColorFormats.data();
	gbufferRenderPassCreateInfo.colorFormatCount = static_cast<uint32_t>(gbufferColorFormats.size());
	gbufferRenderPassCreateInfo.depthFormat = DepthFormat::D24_STENCIL_8;
	gbufferRenderPass = graphicsCore->CreateRenderPass(gbufferRenderPassCreateInfo);

	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(gbufferRenderPassCreateInfo.depthFormat, width, height, false, false, false, "GBuffer Depth Image");

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		imageSet.gbufferRenderTargets.reserve(gbufferColorFormats.size());
		for (size_t i = 0; i < gbufferColorFormats.size(); ++i) {
			RenderTarget::CreateInfo gbufferRtCreateInfo{ gbufferColorFormats[i], width, height, true, gbufferColorAttachmentNames[i] };
			imageSet.gbufferRenderTargets.emplace_back(graphicsCore->CreateRenderTarget(gbufferRtCreateInfo));
		}

		imageSet.gbufferDepthTarget = graphicsCore->CreateDepthTarget(gbufferDepthImageCreateInfo);

		Framebuffer::CreateInfo gbufferCreateInfo{};
		gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
		gbufferCreateInfo.renderPass = gbufferRenderPass;
		gbufferCreateInfo.renderTargetLists = imageSet.gbufferRenderTargets.data();
		gbufferCreateInfo.numRenderTargetLists = static_cast<uint32_t>(imageSet.gbufferRenderTargets.size());
		gbufferCreateInfo.depthTarget = imageSet.gbufferDepthTarget;

		imageSet.gbuffer = graphicsCore->CreateFramebuffer(gbufferCreateInfo);
	}
}

void DeferredRenderer::CreateLitHDRFramebuffer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	RenderTarget::CreateInfo litHdrImagesCreateInfo = { Grindstone::GraphicsAPI::ColorFormat::R16G16B16A16, width, height, true, "Lit HDR Color Image" };
	DepthTarget::CreateInfo litHdrDepthImageCreateInfo(DepthFormat::D24_STENCIL_8, width, height, false, false, false, "Lit HDR Depth Image");

	RenderPass::CreateInfo mainRenderPassCreateInfo{};
	mainRenderPassCreateInfo.debugName = "Main HDR Render Pass";
	mainRenderPassCreateInfo.width = width;
	mainRenderPassCreateInfo.height = height;
	mainRenderPassCreateInfo.colorFormats = &litHdrImagesCreateInfo.format;
	mainRenderPassCreateInfo.colorFormatCount = 1;
	mainRenderPassCreateInfo.depthFormat = litHdrDepthImageCreateInfo.format;
	mainRenderPass = graphicsCore->CreateRenderPass(mainRenderPassCreateInfo);

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		imageSet.litHdrRenderTarget = graphicsCore->CreateRenderTarget(litHdrImagesCreateInfo);
		imageSet.litHdrDepthTarget = graphicsCore->CreateDepthTarget(litHdrDepthImageCreateInfo);

		std::string framebufferName = std::string("Main HDR Framebuffer ") + std::to_string(i);
		Framebuffer::CreateInfo litHdrFramebufferCreateInfo{};
		litHdrFramebufferCreateInfo.debugName = framebufferName.c_str();
		litHdrFramebufferCreateInfo.renderTargetLists = &imageSet.litHdrRenderTarget;
		litHdrFramebufferCreateInfo.numRenderTargetLists = 1;
		litHdrFramebufferCreateInfo.depthTarget = imageSet.litHdrDepthTarget;
		litHdrFramebufferCreateInfo.renderPass = mainRenderPass;
		imageSet.litHdrFramebuffer = graphicsCore->CreateFramebuffer(litHdrFramebufferCreateInfo);
	}
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
		if (!assetManager->LoadShaderSet(Uuid("5537b925-96bc-4e1f-8e2a-d66d6dd9bed1"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load point light shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 2> pointLightLayouts{};
		pointLightLayouts[0] = lightingDescriptorSetLayout;
		pointLightLayouts[1] = lightingUBODescriptorSetLayout;

		pipelineCreateInfo.shaderName = "Point Light Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = pointLightLayouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(pointLightLayouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		pipelineCreateInfo.blendMode = BlendMode::Additive;
		pipelineCreateInfo.renderPass = mainRenderPass;
		pointLightPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(Uuid("31cc60ab-59cb-43b5-94bb-3951844c8f76"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load spot light shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 2> spotLightLayouts{};
		spotLightLayouts[0] = lightingDescriptorSetLayout;
		spotLightLayouts[1] = shadowMappedLightDescriptorSetLayout;

		pipelineCreateInfo.shaderName = "Spot Light Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = spotLightLayouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(spotLightLayouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		pipelineCreateInfo.blendMode = BlendMode::Additive;
		pipelineCreateInfo.renderPass = mainRenderPass;
		spotLightPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(Uuid("94dcc829-3b58-45fb-809a-6800a23eab45"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load directional light shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 2> directionalLightLayouts{};
		directionalLightLayouts[0] = lightingDescriptorSetLayout;
		directionalLightLayouts[1] = shadowMappedLightDescriptorSetLayout;

		pipelineCreateInfo.shaderName = "Directional Light Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = directionalLightLayouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(directionalLightLayouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		pipelineCreateInfo.blendMode = BlendMode::Additive;
		pipelineCreateInfo.renderPass = mainRenderPass;
		directionalLightPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
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
		pipelineCreateInfo.isDepthWriteEnabled = true;
		pipelineCreateInfo.isDepthTestEnabled = true;
		pipelineCreateInfo.isStencilEnabled = true;
		tonemapPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(
			Uuid("fc5b427f-7847-419d-a34d-2a55778eccbd"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load shadow mapping shaders.");
			return;
		}

		Grindstone::GraphicsAPI::VertexBufferLayout shadowMapPositionLayout = {
		{
			0,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexPosition",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
		};

		auto wgb = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

		pipelineCreateInfo.width = 600;
		pipelineCreateInfo.height = 600;
		pipelineCreateInfo.scissorW = 600;
		pipelineCreateInfo.scissorH = 600;

		pipelineCreateInfo.vertexBindings = &shadowMapPositionLayout;
		pipelineCreateInfo.vertexBindingsCount = 1;
		pipelineCreateInfo.shaderName = "Shadow Mapping Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = &shadowMapDescriptorSetLayout;
		pipelineCreateInfo.descriptorSetLayoutCount = 1;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = shadowMapRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = true;
		pipelineCreateInfo.isDepthTestEnabled = true;
		pipelineCreateInfo.isStencilEnabled = false;
		shadowMappingPipeline = graphicsCore->CreatePipeline(pipelineCreateInfo);
	}
}

void DeferredRenderer::RenderLightsImmediate(entt::registry& registry) {
#if 0
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
#endif
}

bool showPoint = true;
bool showSpot = true;
bool showDirect = true;

void DeferredRenderer::RenderLightsCommandBuffer(
	uint32_t imageIndex,
	GraphicsAPI::CommandBuffer* currentCommandBuffer,
	entt::registry& registry
) {
	GRIND_PROFILE_FUNC();
	if (pointLightPipeline == nullptr) {
		return;
	}

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto& imageSet = deferredRendererImageSets[imageIndex];

	ClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 0.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;
	currentCommandBuffer->BindRenderPass(mainRenderPass, imageSet.litHdrFramebuffer, 800, 600, &clearColor, 1, clearDepthStencil);

	currentCommandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	currentCommandBuffer->BindIndexBuffer(indexBuffer);

	const glm::mat4 bias = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);

	if (showPoint) {
		// Point Lights
		currentCommandBuffer->BindPipeline(pointLightPipeline);

		std::array<GraphicsAPI::DescriptorSet*, 2> pointLightDescriptors{};
		pointLightDescriptors[0] = imageSet.lightingDescriptorSet;

		auto view = registry.view<const TransformComponent, PointLightComponent>();
		view.each([&](const TransformComponent& transformComponent, PointLightComponent& pointLightComponent) {
			PointLightComponent::UniformStruct lightmapStruct{
				pointLightComponent.color,
				pointLightComponent.attenuationRadius,
				transformComponent.position,
				pointLightComponent.intensity
			};

			pointLightDescriptors[1] = pointLightComponent.descriptorSet;
			pointLightComponent.uniformBufferObject->UpdateBuffer(&lightmapStruct);
			currentCommandBuffer->BindDescriptorSet(pointLightPipeline, pointLightDescriptors.data(), static_cast<uint32_t>(pointLightDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}

	if (showSpot) {
		// Spot Lights
		currentCommandBuffer->BindPipeline(spotLightPipeline);

		std::array<GraphicsAPI::DescriptorSet*, 2> spotLightDescriptors{};
		spotLightDescriptors[0] = imageSet.lightingDescriptorSet;

		auto view = registry.view<const TransformComponent, SpotLightComponent>();
		view.each([&](const TransformComponent& transformComponent, SpotLightComponent& spotLightComponent) {
			SpotLightComponent::UniformStruct lightStruct {
				bias * spotLightComponent.shadowMatrix,
				spotLightComponent.color,
				spotLightComponent.attenuationRadius,
				transformComponent.position,
				spotLightComponent.intensity,
				transformComponent.GetForward(),
				spotLightComponent.innerAngle,
				spotLightComponent.outerAngle,
				spotLightComponent.shadowResolution
			};

			spotLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

			spotLightDescriptors[1] = spotLightComponent.descriptorSet;
			currentCommandBuffer->BindDescriptorSet(spotLightPipeline, spotLightDescriptors.data(), static_cast<uint32_t>(spotLightDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}

	if (showDirect) {
		// Directional Lights
		currentCommandBuffer->BindPipeline(directionalLightPipeline);

		std::array<GraphicsAPI::DescriptorSet*, 2> directionalLightDescriptors{};
		directionalLightDescriptors[0] = imageSet.lightingDescriptorSet;

		auto view = registry.view<const TransformComponent, DirectionalLightComponent>();
		view.each([&](const TransformComponent& transformComponent, DirectionalLightComponent& directionalLightComponent) {
			DirectionalLightComponent::UniformStruct lightStruct{
				bias * directionalLightComponent.shadowMatrix,
				directionalLightComponent.color,
				directionalLightComponent.sourceRadius,
				transformComponent.GetForward(),
				directionalLightComponent.intensity,
				directionalLightComponent.shadowResolution
			};

			directionalLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

			directionalLightDescriptors[1] = directionalLightComponent.descriptorSet;
			currentCommandBuffer->BindDescriptorSet(directionalLightPipeline, directionalLightDescriptors.data(), static_cast<uint32_t>(directionalLightDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}

	currentCommandBuffer->UnbindRenderPass();
}

void DeferredRenderer::RenderShadowMaps(CommandBuffer* commandBuffer, entt::registry& registry) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto assetManager = EngineCore::GetInstance().assetRendererManager;

	ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;

	if (showSpot) {
		auto view = registry.view<const TransformComponent, SpotLightComponent>();
		view.each([&](const TransformComponent& transformComponent, SpotLightComponent& spotLightComponent) {
			float fov = glm::radians(90.0f); //spotLightComponent.outerAngle * 2.0f;
			float farDist = spotLightComponent.attenuationRadius;

			const glm::vec3 forwardVector = transformComponent.GetForward();
			const glm::vec3 pos = transformComponent.position;

			const auto viewMatrix = glm::lookAt(
				pos,
				pos + forwardVector,
				transformComponent.GetUp()
			);

			auto projectionMatrix = glm::perspective(
				fov,
				1.0f,
				0.1f,
				farDist
			);

			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			spotLightComponent.shadowMatrix = projectionMatrix * viewMatrix;
			glm::mat4 shadowPass = spotLightComponent.shadowMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			spotLightComponent.shadowMatrix = spotLightComponent.shadowMatrix * glm::mat4(1.0f);

			uint32_t resolution = static_cast<uint32_t>(spotLightComponent.shadowResolution);

			spotLightComponent.shadowMapUniformBufferObject->UpdateBuffer(&shadowPass);

			commandBuffer->BindRenderPass(
				spotLightComponent.renderPass,
				spotLightComponent.framebuffer,
				resolution,
				resolution,
				nullptr,
				0,
				clearDepthStencil
			);

			float resF = static_cast<float>(resolution);
			commandBuffer->BindPipeline(shadowMappingPipeline);

			// commandBuffer->SetDepthBias(1.25f, 1.75f);

			commandBuffer->BindDescriptorSet(shadowMappingPipeline, &spotLightComponent.shadowMapDescriptorSet, 1);
			assetManager->RenderShadowMap(commandBuffer, spotLightComponent.shadowMapDescriptorSet);

			commandBuffer->UnbindRenderPass();
		});
	}

	if (showDirect) {
		auto view = registry.view<const TransformComponent, DirectionalLightComponent>();
		view.each([&](const TransformComponent& transformComponent, DirectionalLightComponent& directionalLightComponent) {
			const float shadowHalfSize = 40.0f;
			glm::mat4 projectionMatrix = glm::ortho<float>(-shadowHalfSize, shadowHalfSize, -shadowHalfSize, shadowHalfSize, 0, 80);
			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			glm::mat4 viewMatrix = glm::lookAt(
				transformComponent.GetForward() * -70.0f,
				glm::vec3(0, 0, 0),
				transformComponent.GetUp()
			);

			glm::mat4 projView = projectionMatrix * viewMatrix;
			glm::mat4 shadowPass = projView * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			directionalLightComponent.shadowMatrix = projView * glm::mat4(1.0f);

			uint32_t resolution = static_cast<uint32_t>(directionalLightComponent.shadowResolution);

			directionalLightComponent.shadowMapUniformBufferObject->UpdateBuffer(&shadowPass);

			commandBuffer->BindRenderPass(
				directionalLightComponent.renderPass,
				directionalLightComponent.framebuffer,
				resolution,
				resolution,
				nullptr,
				0,
				clearDepthStencil
			);

			commandBuffer->BindPipeline(shadowMappingPipeline);

			commandBuffer->BindDescriptorSet(shadowMappingPipeline, &directionalLightComponent.shadowMapDescriptorSet, 1);
			assetManager->RenderShadowMap(commandBuffer, directionalLightComponent.shadowMapDescriptorSet);

			commandBuffer->UnbindRenderPass();
		});
	}
}

void DeferredRenderer::PostProcessImmediate(GraphicsAPI::Framebuffer* outputFramebuffer) {

}

void DeferredRenderer::PostProcessCommandBuffer(
	uint32_t imageIndex,
	GraphicsAPI::Framebuffer* framebuffer,
	GraphicsAPI::CommandBuffer* currentCommandBuffer
) {
	GRIND_PROFILE_FUNC();

	auto& imageSet = deferredRendererImageSets[imageIndex];
	
	ClearColorValue clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = false;
	currentCommandBuffer->BindRenderPass(framebuffer->GetRenderPass(), framebuffer, 800, 600, &clearColor, 1, clearDepthStencil);
	
	currentCommandBuffer->BindDescriptorSet(tonemapPipeline, &imageSet.tonemapDescriptorSet, 1);
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
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	if (graphicsCore->ShouldUseImmediateMode()) {
		RenderImmediate(
			registry,
			projectionMatrix,
			viewMatrix,
			eyePos,
			outputFramebuffer
		);
	}
	else {
		RenderCommandBuffer(
			registry,
			projectionMatrix,
			viewMatrix,
			eyePos,
			outputFramebuffer
		);
	}
}

void DeferredRenderer::RenderCommandBuffer(
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto assetManager = EngineCore::GetInstance().assetRendererManager;
	auto wgb = EngineCore::GetInstance().windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	wgb->AcquireNextImage();

	GraphicsAPI::Framebuffer* targetFramebuffer = outputFramebuffer;
	if (outputFramebuffer == nullptr) {
		targetFramebuffer = wgb->GetCurrentFramebuffer();
	}

	uint32_t imageIndex = wgb->GetCurrentImageIndex();
	auto& imageSet = deferredRendererImageSets[imageIndex];

	EngineUboStruct engineUboStruct{};
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.eyePos = eyePos;
	imageSet.globalUniformBufferObject->UpdateBuffer(&engineUboStruct);

	auto currentCommandBuffer = imageSet.commandBuffer;
	currentCommandBuffer->BeginCommandBuffer();

	RenderShadowMaps(currentCommandBuffer, registry);

	ClearColorValue clearColors[] = {
		ClearColorValue{0.3f, 0.6f, 0.9f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
		ClearColorValue{0.0f, 0.0f, 0.0f, 1.f}
	};

	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;
	assetManager->SetEngineDescriptorSet(imageSet.engineDescriptorSet);

	currentCommandBuffer->BindRenderPass(gbufferRenderPass, imageSet.gbuffer, 800, 600, clearColors, 4, clearDepthStencil);
	assetManager->RenderQueue(currentCommandBuffer, "Opaque");
	currentCommandBuffer->UnbindRenderPass();
	RenderLightsCommandBuffer(imageIndex, currentCommandBuffer, registry);
	// assetManager->RenderQueue("Unlit");
	// assetManager->RenderQueue("Transparent");

	PostProcessCommandBuffer(imageIndex, targetFramebuffer, currentCommandBuffer);

	currentCommandBuffer->EndCommandBuffer();
	wgb->SubmitCommandBuffer(currentCommandBuffer);
	wgb->PresentSwapchain();
}

void DeferredRenderer::RenderImmediate(
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
#if 0
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
#endif
}
