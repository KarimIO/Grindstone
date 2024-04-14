#include <array>
#include <random>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Common/Graphics/Core.hpp"
#include "Common/Graphics/VertexArrayObject.hpp"
#include "Common/Graphics/GraphicsPipeline.hpp"
#include "DeferredRenderer.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "EngineCore/CoreComponents/EnvironmentMap/EnvironmentMapComponent.hpp"
#include "EngineCore/CoreComponents/Lights/PointLightComponent.hpp"
#include "EngineCore/CoreComponents/Lights/SpotLightComponent.hpp"
#include "EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp"
#include "EngineCore/AssetRenderer/AssetRendererManager.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "Common/Event/WindowEvent.hpp"
#include "EngineCore/Profiling.hpp"
#include <Common/Window/WindowManager.hpp>
#include <Common/HashedString.hpp>
using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

GraphicsAPI::RenderPass* DeferredRenderer::gbufferRenderPass = nullptr;
GraphicsAPI::RenderPass* DeferredRenderer::mainRenderPass = nullptr;

const size_t MAX_BLOOM_MIPS = 40u;

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
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 eyePos;
};

const size_t ssaoKernelSize = 64;
struct SsaoUboStruct {
	glm::vec4 kernels[ssaoKernelSize];
	float radius;
	float bias;
};

enum class BloomStage : uint32_t {
	Filter = 0,
	Downsample,
	Upsample
};

struct BloomUboStruct {
	glm::vec4 thresholdFilter; // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
	BloomStage stage;
	float levelOfDetail;
	float filterRadius;
};

static size_t CalculateBloomLevels(uint32_t width, uint32_t height) {
	float minDimension = static_cast<float>(glm::min(width, height));
	float logDimension = glm::log2(minDimension);
	return glm::min(static_cast<size_t>(logDimension) - 1, MAX_BLOOM_MIPS);
}

DeferredRenderer::DeferredRenderer(GraphicsAPI::RenderPass* targetRenderPass) : targetRenderPass(targetRenderPass) {
	width = 800;
	height = 600;

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();
	deferredRendererImageSets.resize(maxFramesInFlight);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.colorFormats = nullptr;
	renderPassCreateInfo.colorFormatCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	shadowMapRenderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	bloomMipLevelCount = CalculateBloomLevels(width, height);

	Uuid brdfAssetUuid("7707483a-9379-4e81-9e15-0e5acf20e9d6");
	const TextureAsset* textureAsset = engineCore.assetManager->GetAsset<TextureAsset>(brdfAssetUuid);
	brdfLut = textureAsset != nullptr
		? textureAsset->texture
		: nullptr;

	CreateSsaoKernelAndNoise();
	CreateVertexAndIndexBuffersAndLayouts();
	CreateGbufferFramebuffer();
	CreateLitHDRFramebuffer();
	CreateUniformBuffers();
	CreateDescriptorSetLayouts();
	CreateBloomResources();
	CreateBloomUniformBuffers();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		CreateBloomRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateDescriptorSets(deferredRendererImageSets[i]);
	}

	CreatePipelines();
}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	CleanupPipelines();

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
		// graphicsCore->DeleteDepthTarget(imageSet.litHdrDepthTarget);

		graphicsCore->DeleteDescriptorSet(imageSet.engineDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.tonemapDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.lightingDescriptorSet);
	}

	graphicsCore->DeleteDescriptorSetLayout(engineDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(tonemapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingDescriptorSetLayout);

	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);

	graphicsCore->DeleteUniformBuffer(ssaoUniformBuffer);
	graphicsCore->DeleteTexture(ssaoNoiseTexture);
	graphicsCore->DeleteDescriptorSetLayout(ssaoDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSet(ssaoDescriptorSet);
}

void DeferredRenderer::CreateBloomResources() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	{
		std::array<DescriptorSetLayout::Binding, 4> bloomLayoutBindings{};
		bloomLayoutBindings[0].bindingId = 0;
		bloomLayoutBindings[0].count = 1;
		bloomLayoutBindings[0].type = BindingType::UniformBuffer;
		bloomLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Compute;

		bloomLayoutBindings[1].bindingId = 1;
		bloomLayoutBindings[1].count = 1;
		bloomLayoutBindings[1].type = BindingType::RenderTextureStorageImage;
		bloomLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Compute;

		bloomLayoutBindings[2].bindingId = 2;
		bloomLayoutBindings[2].count = 1;
		bloomLayoutBindings[2].type = BindingType::RenderTexture;
		bloomLayoutBindings[2].stages = GraphicsAPI::ShaderStageBit::Compute;

		bloomLayoutBindings[3].bindingId = 3;
		bloomLayoutBindings[3].count = 1;
		bloomLayoutBindings[3].type = BindingType::RenderTexture;
		bloomLayoutBindings[3].stages = GraphicsAPI::ShaderStageBit::Compute;

		DescriptorSetLayout::CreateInfo bloomDescriptorSetLayoutCreateInfo{};
		bloomDescriptorSetLayoutCreateInfo.debugName = "Bloom Descriptor Set Layout";
		bloomDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bloomLayoutBindings.size());
		bloomDescriptorSetLayoutCreateInfo.bindings = bloomLayoutBindings.data();
		bloomDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(bloomDescriptorSetLayoutCreateInfo);
	}
}

void DeferredRenderer::CreateSsaoKernelAndNoise() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	{
		SsaoUboStruct ssaoUboStruct{};
		ssaoUboStruct.bias = 0.025f;
		ssaoUboStruct.radius = 0.5f;

		for (size_t i = 0; i < ssaoKernelSize; ++i)
		{
			glm::vec4 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator),
				0.0f
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			ssaoUboStruct.kernels[i] = sample;
		}

		GraphicsAPI::UniformBuffer::CreateInfo ssaoUniformBufferObjectCi{};
		ssaoUniformBufferObjectCi.debugName = "SSAO Uniform Buffer";
		ssaoUniformBufferObjectCi.isDynamic = false;
		ssaoUniformBufferObjectCi.size = sizeof(SsaoUboStruct);
		ssaoUniformBuffer = graphicsCore->CreateUniformBuffer(ssaoUniformBufferObjectCi);
		ssaoUniformBuffer->UpdateBuffer(&ssaoUboStruct);
	}

	{
		constexpr size_t ssaoNoiseDimSize = 4;
		std::array<glm::vec2, ssaoNoiseDimSize * ssaoNoiseDimSize> ssaoNoise{};
		for (size_t i = 0; i < ssaoNoise.size(); i++) {
			ssaoNoise[i] = glm::vec2(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0
			);
		}

		GraphicsAPI::Texture::CreateInfo ssaoNoiseCreateInfo{};
		ssaoNoiseCreateInfo.debugName = "SSAO Noise Texture";
		ssaoNoiseCreateInfo.data = reinterpret_cast<const char*>(ssaoNoise.data());
		ssaoNoiseCreateInfo.size = static_cast<uint32_t>(sizeof(ssaoNoise));
		ssaoNoiseCreateInfo.format = GraphicsAPI::ColorFormat::RG32;
		ssaoNoiseCreateInfo.width = ssaoNoiseCreateInfo.height = ssaoNoiseDimSize;
		ssaoNoiseCreateInfo.isCubemap = false;
		ssaoNoiseCreateInfo.mipmaps = 1;
		ssaoNoiseCreateInfo.options.shouldGenerateMipmaps = false;
		ssaoNoiseCreateInfo.options.magFilter = TextureFilter::Nearest;
		ssaoNoiseCreateInfo.options.minFilter = TextureFilter::Nearest;
		ssaoNoiseCreateInfo.options.wrapModeU = TextureWrapMode::Repeat;
		ssaoNoiseCreateInfo.options.wrapModeV = TextureWrapMode::Repeat;
		ssaoNoiseCreateInfo.options.wrapModeW = TextureWrapMode::Repeat;
		ssaoNoiseTexture = graphicsCore->CreateTexture(ssaoNoiseCreateInfo);
	}

	{
		std::array<DescriptorSetLayout::Binding, 2> ssaoLayoutBindings{};
		ssaoLayoutBindings[0].bindingId = 0;
		ssaoLayoutBindings[0].count = 1;
		ssaoLayoutBindings[0].type = BindingType::UniformBuffer;
		ssaoLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoLayoutBindings[1].bindingId = 1;
		ssaoLayoutBindings[1].count = 1;
		ssaoLayoutBindings[1].type = BindingType::Texture;
		ssaoLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		DescriptorSetLayout::CreateInfo ssaoDescriptorSetLayoutCreateInfo{};
		ssaoDescriptorSetLayoutCreateInfo.debugName = "SSAO Descriptor Set Layout";
		ssaoDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		ssaoDescriptorSetLayoutCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssaoDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<DescriptorSet::Binding, 2> ssaoLayoutBindings{};
		ssaoLayoutBindings[0].bindingIndex = 0;
		ssaoLayoutBindings[0].count = 1;
		ssaoLayoutBindings[0].bindingType = BindingType::UniformBuffer;
		ssaoLayoutBindings[0].itemPtr = ssaoUniformBuffer;

		ssaoLayoutBindings[1].bindingIndex = 1;
		ssaoLayoutBindings[1].count = 1;
		ssaoLayoutBindings[1].bindingType = BindingType::Texture;
		ssaoLayoutBindings[1].itemPtr = ssaoNoiseTexture;

		DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
		engineDescriptorSetCreateInfo.debugName = "SSAO Descriptor Set";
		engineDescriptorSetCreateInfo.layout = ssaoDescriptorSetLayout;
		engineDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		engineDescriptorSetCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);
	}

	{
		uint32_t halfWidth = width / 2;
		uint32_t halfHeight = height / 2;
		GraphicsAPI::RenderTarget::CreateInfo ssaoRenderTargetCreateInfo{ GraphicsAPI::ColorFormat::R8, halfWidth, halfHeight, true, false, "SSAO Render Target" };
		ssaoRenderTarget = graphicsCore->CreateRenderTarget(ssaoRenderTargetCreateInfo);

		GraphicsAPI::RenderPass::CreateInfo ssaoRenderPassCreateInfo{};
		ssaoRenderPassCreateInfo.debugName = "SSAO Renderpass";
		ssaoRenderPassCreateInfo.colorFormats = &ssaoRenderTargetCreateInfo.format;
		ssaoRenderPassCreateInfo.colorFormatCount = 1;
		ssaoRenderPassCreateInfo.depthFormat = DepthFormat::None;
		ssaoRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		ssaoRenderPass = graphicsCore->CreateRenderPass(ssaoRenderPassCreateInfo);

		GraphicsAPI::Framebuffer::CreateInfo ssaoFramebufferCreateInfo{};
		ssaoFramebufferCreateInfo.debugName = "SSAO Framebuffer";
		ssaoFramebufferCreateInfo.width = halfWidth;
		ssaoFramebufferCreateInfo.height = halfHeight;
		ssaoFramebufferCreateInfo.renderTargetLists = &ssaoRenderTarget;
		ssaoFramebufferCreateInfo.numRenderTargetLists = 1;
		ssaoFramebufferCreateInfo.depthTarget = nullptr;
		ssaoFramebufferCreateInfo.renderPass = ssaoRenderPass;
		ssaoFramebuffer = graphicsCore->CreateFramebuffer(ssaoFramebufferCreateInfo);
	}
}

void DeferredRenderer::CleanupPipelines() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->DeleteGraphicsPipeline(pointLightPipeline);
	graphicsCore->DeleteGraphicsPipeline(spotLightPipeline);
	graphicsCore->DeleteGraphicsPipeline(directionalLightPipeline);
	graphicsCore->DeleteGraphicsPipeline(shadowMappingPipeline);
	graphicsCore->DeleteGraphicsPipeline(tonemapPipeline);

	pointLightPipeline = nullptr;
	spotLightPipeline = nullptr;
	directionalLightPipeline = nullptr;
	shadowMappingPipeline = nullptr;
	tonemapPipeline = nullptr;
}

bool DeferredRenderer::OnWindowResize(Events::BaseEvent* ev) {
	if (ev->GetEventType() == Events::EventType::WindowResize) {
		Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
		Resize(winResizeEvent->width, winResizeEvent->height);
	}

	return false;
}

void DeferredRenderer::Resize(uint32_t width, uint32_t height) {
	if (this->width == width && this->height == height) {
		return;
	}

	this->width = width;
	this->height = height;

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->WaitUntilIdle();

	uint32_t halfWidth = width / 2;
	uint32_t halfHeight = height / 2;

	ssaoRenderTarget->Resize(halfWidth, halfHeight);
	ssaoFramebuffer->Resize(halfWidth, halfHeight);

	bloomMipLevelCount = CalculateBloomLevels(width, height);

	CreateBloomUniformBuffers();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		DeferredRendererImageSet& imageSet = deferredRendererImageSets[i];

		for (GraphicsAPI::RenderTarget* gbufferRenderTarget : imageSet.gbufferRenderTargets) {
			gbufferRenderTarget->Resize(width, height);
		}

		imageSet.gbufferDepthTarget->Resize(width, height);
		imageSet.gbuffer->Resize(width, height);
		imageSet.litHdrRenderTarget->Resize(width, height);
		imageSet.litHdrFramebuffer->Resize(width, height);

		CleanupPipelines();
		CreatePipelines();

		CreateBloomRenderTargetsAndDescriptorSets(imageSet, i);
		UpdateDescriptorSets(imageSet);
	}
}

void DeferredRenderer::CreateBloomUniformBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	for (size_t i = 0; i < bloomUniformBuffers.size(); ++i) {
		if (bloomUniformBuffers[i] != nullptr) {
			graphicsCore->DeleteUniformBuffer(bloomUniformBuffers[i]);
		}
	}

	bloomUniformBuffers.resize(bloomMipLevelCount * 2);

	GraphicsAPI::UniformBuffer::CreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.debugName = "Bloom Uniform Buffer";
	uniformBufferCreateInfo.isDynamic = true;
	uniformBufferCreateInfo.size = sizeof(BloomUboStruct);

	for (size_t i = 0; i < bloomUniformBuffers.size(); ++i) {
		bloomUniformBuffers[i] = graphicsCore->CreateUniformBuffer(uniformBufferCreateInfo);
	}
}

void DeferredRenderer::CreateBloomRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	for (size_t i = 0; i < imageSet.bloomRenderTargets.size(); ++i) {
		if (imageSet.bloomRenderTargets[i] != nullptr) {
			graphicsCore->DeleteRenderTarget(imageSet.bloomRenderTargets[i]);
		}
	}

	for (size_t i = 0; i < imageSet.bloomDescriptorSets.size(); ++i) {
		if (imageSet.bloomDescriptorSets[i] != nullptr) {
			graphicsCore->DeleteDescriptorSet(imageSet.bloomDescriptorSets[i]);
		}
	}

	imageSet.bloomRenderTargets.resize(bloomMipLevelCount * 2);
	imageSet.bloomDescriptorSets.resize(bloomMipLevelCount * 2 - 2);

	GraphicsAPI::RenderTarget::CreateInfo bloomRenderTargetCreateInfo{ GraphicsAPI::ColorFormat::RGBA32, width, height, true, true, "Bloom Render Target" };

	for (uint32_t i = 0; i < bloomMipLevelCount; ++i) {
		std::string bloomRenderTargetName = std::string("Bloom Render Target Downscale Mip ") + std::to_string(i);
		bloomRenderTargetCreateInfo.debugName = bloomRenderTargetName.c_str();
		imageSet.bloomRenderTargets[i] = graphicsCore->CreateRenderTarget(bloomRenderTargetCreateInfo);
		bloomRenderTargetCreateInfo.width = bloomRenderTargetCreateInfo.width / 2;
		bloomRenderTargetCreateInfo.height = bloomRenderTargetCreateInfo.height / 2;
	}

	bloomRenderTargetCreateInfo.width = width;
	bloomRenderTargetCreateInfo.height = height;

	for (uint32_t i = 0; i < bloomMipLevelCount; ++i) {
		std::string bloomRenderTargetName = std::string("Bloom Render Target Upscale Mip ") + std::to_string(i);
		bloomRenderTargetCreateInfo.debugName = bloomRenderTargetName.c_str();
		imageSet.bloomRenderTargets[bloomMipLevelCount + i] = graphicsCore->CreateRenderTarget(bloomRenderTargetCreateInfo);
		bloomRenderTargetCreateInfo.width = bloomRenderTargetCreateInfo.width / 2;
		bloomRenderTargetCreateInfo.height = bloomRenderTargetCreateInfo.height / 2;
	}

	std::array<GraphicsAPI::DescriptorSet::Binding, 4> descriptorBindings;
	descriptorBindings[0].bindingIndex = 0;
	descriptorBindings[0].bindingType = BindingType::UniformBuffer;
	descriptorBindings[0].count = 1;

	descriptorBindings[1].bindingIndex = 1;
	descriptorBindings[1].bindingType = BindingType::RenderTextureStorageImage;
	descriptorBindings[1].count = 1;

	descriptorBindings[2].bindingIndex = 2;
	descriptorBindings[2].bindingType = BindingType::RenderTexture;
	descriptorBindings[2].count = 1;

	descriptorBindings[3].bindingIndex = 3;
	descriptorBindings[3].bindingType = BindingType::RenderTexture;
	descriptorBindings[3].count = 1;

	descriptorBindings[3].itemPtr = imageSet.bloomRenderTargets[0];

	// Threshold values sourced from: https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/
	float threshold = 1.0f;
	float softThreshold = 0.5f;
	float knee = threshold * softThreshold;
	float thresholdBias = 0.00001f;

	BloomUboStruct bloomUboStruct{};
	bloomUboStruct.thresholdFilter = { threshold, threshold - knee, 2.0f * knee, 0.25f / (knee + thresholdBias) };
	bloomUboStruct.levelOfDetail = 0.0f;
	bloomUboStruct.filterRadius = 0.005f;

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.layout = bloomDescriptorSetLayout;
	descriptorSetCreateInfo.debugName = "Bloom Descriptor Set";
	descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
	descriptorSetCreateInfo.bindings = descriptorBindings.data();

	uint32_t bloomDescriptorSetIndex = 0;

	bloomUboStruct.stage = BloomStage::Filter;
	if (bloomMipLevelCount > 1) {
		std::string bloomDescriptorName = fmt::format("Bloom DS Filter [{}]", imageSetIndex);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[1];
		descriptorBindings[2].itemPtr = imageSet.litHdrRenderTarget;
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Downsample;
	for (size_t i = 1; i < bloomMipLevelCount - 1; ++i) {
		std::string bloomDescriptorName = fmt::format("Bloom DS Downsample [{}]({})", imageSetIndex, i);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[i + 1];
		descriptorBindings[2].itemPtr = imageSet.bloomRenderTargets[i];
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		std::string bloomDescriptorName = fmt::format("Bloom DS Upsample [{}]({})", imageSetIndex, bloomMipLevelCount - 1);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount * 2 - 1];
		descriptorBindings[3].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount - 1];
		descriptorBindings[2].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount - 2];
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Upsample;
	for (size_t i = bloomMipLevelCount - 2; i >= 1; --i) {
		std::string bloomDescriptorName = fmt::format("Bloom DS Upsample [{}]({})", imageSetIndex, i);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount + i];
		descriptorBindings[3].itemPtr = imageSet.bloomRenderTargets[i];
		descriptorBindings[2].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount + i + 1];
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
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

	CreateDescriptorSetLayouts();
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
	// TODO: Just using vertex for now to get resolution cuz I'm lazy. Remove this eventually and use a uniform buffer.
	litHdrRenderTargetBinding.stages = ShaderStageBit::Vertex | ShaderStageBit::Fragment;

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

	DescriptorSetLayout::Binding bloomBindingForTonemap{};
	bloomBindingForTonemap.bindingId = 2;
	bloomBindingForTonemap.count = 1;
	bloomBindingForTonemap.type = BindingType::RenderTexture;
	bloomBindingForTonemap.stages = ShaderStageBit::Fragment;

	std::array<DescriptorSetLayout::Binding, 3> tonemapDescriptorSetLayoutBindings{};
	tonemapDescriptorSetLayoutBindings[0] = engineUboBinding;
	tonemapDescriptorSetLayoutBindings[1] = litHdrRenderTargetBinding;
	tonemapDescriptorSetLayoutBindings[2] = bloomBindingForTonemap;

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

	DescriptorSet::Binding bloomBindingForTonemap{};
	bloomBindingForTonemap.bindingIndex = 2;
	bloomBindingForTonemap.count = 1;
	bloomBindingForTonemap.bindingType = BindingType::RenderTexture;
	bloomBindingForTonemap.itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount + 1];

	std::array<DescriptorSet::Binding, 3> tonemapDescriptorSetBindings{};
	tonemapDescriptorSetBindings[0] = engineUboBinding;
	tonemapDescriptorSetBindings[1] = litHdrRenderTargetBinding;
	tonemapDescriptorSetBindings[2] = bloomBindingForTonemap;

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

	{
		std::array<DescriptorSetLayout::Binding, 2> ssaoInputLayoutBinding{};
		ssaoInputLayoutBinding[0].bindingId = 0;
		ssaoInputLayoutBinding[0].count = 1;
		ssaoInputLayoutBinding[0].type = BindingType::RenderTexture;
		ssaoInputLayoutBinding[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoInputLayoutBinding[1].bindingId = 1;
		ssaoInputLayoutBinding[1].count = 1;
		ssaoInputLayoutBinding[1].type = BindingType::Texture;
		ssaoInputLayoutBinding[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		DescriptorSetLayout::CreateInfo ssaoInputLayoutCreateInfo{};
		ssaoInputLayoutCreateInfo.debugName = "SSAO Descriptor Set Layout";
		ssaoInputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssaoInputLayoutBinding.size());
		ssaoInputLayoutCreateInfo.bindings = ssaoInputLayoutBinding.data();
		ssaoInputDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssaoInputLayoutCreateInfo);
	}

	{
		std::array<DescriptorSet::Binding, 2> ssaoInputBinding;
		ssaoInputBinding[0].bindingIndex = 0;
		ssaoInputBinding[0].count = 1;
		ssaoInputBinding[0].bindingType = BindingType::RenderTexture;
		ssaoInputBinding[0].itemPtr = ssaoRenderTarget;

		ssaoInputBinding[1].bindingIndex = 1;
		ssaoInputBinding[1].count = 1;
		ssaoInputBinding[1].bindingType = BindingType::Texture;
		ssaoInputBinding[1].itemPtr = brdfLut;

		DescriptorSet::CreateInfo ssaoInputCreateInfo{};
		ssaoInputCreateInfo.debugName = "SSAO Descriptor Set";
		ssaoInputCreateInfo.layout = ssaoInputDescriptorSetLayout;
		ssaoInputCreateInfo.bindingCount = static_cast<uint32_t>(ssaoInputBinding.size());
		ssaoInputCreateInfo.bindings = ssaoInputBinding.data();
		ssaoInputDescriptorSet = graphicsCore->CreateDescriptorSet(ssaoInputCreateInfo);
	}

	{
		DescriptorSetLayout::Binding environmentMapLayoutBinding{};
		environmentMapLayoutBinding.bindingId = 0;
		environmentMapLayoutBinding.count = 1;
		environmentMapLayoutBinding.type = BindingType::Texture;
		environmentMapLayoutBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

		DescriptorSetLayout::CreateInfo environmentMapLayoutCreateInfo{};
		environmentMapLayoutCreateInfo.debugName = "Environment Map Input Descriptor Set Layout";
		environmentMapLayoutCreateInfo.bindingCount = 1;
		environmentMapLayoutCreateInfo.bindings = &environmentMapLayoutBinding;
		environmentMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(environmentMapLayoutCreateInfo);
	}

	{
		DescriptorSet::Binding environmentMapBinding{};
		environmentMapBinding.bindingIndex = 0;
		environmentMapBinding.count = 1;
		environmentMapBinding.bindingType = BindingType::RenderTexture;
		environmentMapBinding.itemPtr = ssaoRenderTarget;

		DescriptorSet::CreateInfo environmentMapDescriptorCreateInfo{};
		environmentMapDescriptorCreateInfo.debugName = "Environment Map Input Descriptor Set";
		environmentMapDescriptorCreateInfo.layout = environmentMapDescriptorSetLayout;
		environmentMapDescriptorCreateInfo.bindingCount = 1;
		environmentMapDescriptorCreateInfo.bindings = &environmentMapBinding;
		environmentMapDescriptorSet = graphicsCore->CreateDescriptorSet(environmentMapDescriptorCreateInfo);
	}
}

void DeferredRenderer::UpdateDescriptorSets(DeferredRendererImageSet& imageSet) {
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

	imageSet.engineDescriptorSet->ChangeBindings(&engineUboBinding, 1);

	{
		DescriptorSet::Binding bloomDescriptorSetBinding{};
		bloomDescriptorSetBinding.bindingIndex = 2;
		bloomDescriptorSetBinding.count = 1;
		bloomDescriptorSetBinding.bindingType = BindingType::RenderTexture;
		bloomDescriptorSetBinding.itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount + 1];

		std::array<DescriptorSet::Binding, 3> tonemapDescriptorSetBindings{};
		tonemapDescriptorSetBindings[0] = engineUboBinding;
		tonemapDescriptorSetBindings[1] = litHdrRenderTargetBinding;
		tonemapDescriptorSetBindings[2] = bloomDescriptorSetBinding;

		imageSet.tonemapDescriptorSet->ChangeBindings(tonemapDescriptorSetBindings.data(), static_cast<uint32_t>(tonemapDescriptorSetBindings.size()));
	}

	{
		std::array<DescriptorSet::Binding, 5> lightingDescriptorSetBindings{};
		lightingDescriptorSetBindings[0] = engineUboBinding;
		lightingDescriptorSetBindings[1] = gbuffer0Binding;
		lightingDescriptorSetBindings[2] = gbuffer1Binding;
		lightingDescriptorSetBindings[3] = gbuffer2Binding;
		lightingDescriptorSetBindings[4] = gbuffer3Binding;

		imageSet.lightingDescriptorSet->ChangeBindings(lightingDescriptorSetBindings.data(), static_cast<uint32_t>(lightingDescriptorSetBindings.size()));
	}

	{
		DescriptorSet::Binding ssaoInputBinding{};
		ssaoInputBinding.bindingIndex = 0;
		ssaoInputBinding.count = 1;
		ssaoInputBinding.bindingType = BindingType::RenderTexture;
		ssaoInputBinding.itemPtr = ssaoRenderTarget;

		ssaoInputDescriptorSet->ChangeBindings(&ssaoInputBinding, 1);
	}
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
	gbufferColorFormats[0] = ColorFormat::RGBA16; // X Y Z
	gbufferColorFormats[1] = ColorFormat::RGBA8; // R  G  B matID
	gbufferColorFormats[2] = ColorFormat::RGBA16; // nX nY nZ
	gbufferColorFormats[3] = ColorFormat::RGBA8; // sR sG sB Roughness

	std::array<const char*, gbufferColorCount> gbufferColorAttachmentNames{};
	gbufferColorAttachmentNames[0] = "GBuffer Position Image";
	gbufferColorAttachmentNames[1] = "GBuffer Albedo Image";
	gbufferColorAttachmentNames[2] = "GBuffer Normal Image";
	gbufferColorAttachmentNames[3] = "GBuffer Specular + Roughness Image";

	DepthFormat depthFormat = DepthFormat::D24_STENCIL_8;

	if (gbufferRenderPass == nullptr) {
		RenderPass::CreateInfo gbufferRenderPassCreateInfo{};
		gbufferRenderPassCreateInfo.debugName = "GBuffer Render Pass";
		gbufferRenderPassCreateInfo.colorFormats = gbufferColorFormats.data();
		gbufferRenderPassCreateInfo.colorFormatCount = static_cast<uint32_t>(gbufferColorFormats.size());
		gbufferRenderPassCreateInfo.depthFormat = depthFormat;
		gbufferRenderPass = graphicsCore->CreateRenderPass(gbufferRenderPassCreateInfo);
	}

	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(depthFormat, width, height, false, false, false, "GBuffer Depth Image");

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		imageSet.gbufferRenderTargets.reserve(gbufferColorFormats.size());
		for (size_t i = 0; i < gbufferColorFormats.size(); ++i) {
			RenderTarget::CreateInfo gbufferRtCreateInfo{ gbufferColorFormats[i], width, height, true, false, gbufferColorAttachmentNames[i] };
			imageSet.gbufferRenderTargets.emplace_back(graphicsCore->CreateRenderTarget(gbufferRtCreateInfo));
		}

		imageSet.gbufferDepthTarget = graphicsCore->CreateDepthTarget(gbufferDepthImageCreateInfo);

		Framebuffer::CreateInfo gbufferCreateInfo{};
		gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
		gbufferCreateInfo.width = width;
		gbufferCreateInfo.height = height;
		gbufferCreateInfo.renderPass = gbufferRenderPass;
		gbufferCreateInfo.renderTargetLists = imageSet.gbufferRenderTargets.data();
		gbufferCreateInfo.numRenderTargetLists = static_cast<uint32_t>(imageSet.gbufferRenderTargets.size());
		gbufferCreateInfo.depthTarget = imageSet.gbufferDepthTarget;

		imageSet.gbuffer = graphicsCore->CreateFramebuffer(gbufferCreateInfo);
	}
}

void DeferredRenderer::CreateLitHDRFramebuffer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	RenderTarget::CreateInfo litHdrImagesCreateInfo = { Grindstone::GraphicsAPI::ColorFormat::RGBA16, width, height, true, false, "Lit HDR Color Image" };
	// DepthTarget::CreateInfo litHdrDepthImageCreateInfo(DepthFormat::D24_STENCIL_8, width, height, false, false, false, "Lit HDR Depth Image");

	if (mainRenderPass == nullptr) {
		RenderPass::CreateInfo mainRenderPassCreateInfo{};
		mainRenderPassCreateInfo.debugName = "Main HDR Render Pass";
		mainRenderPassCreateInfo.colorFormats = &litHdrImagesCreateInfo.format;
		mainRenderPassCreateInfo.colorFormatCount = 1;
		mainRenderPassCreateInfo.depthFormat = DepthFormat::D24_STENCIL_8;
		mainRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		mainRenderPass = graphicsCore->CreateRenderPass(mainRenderPassCreateInfo);
	}

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		imageSet.litHdrRenderTarget = graphicsCore->CreateRenderTarget(litHdrImagesCreateInfo);

		std::string framebufferName = std::string("Main HDR Framebuffer ") + std::to_string(i);
		Framebuffer::CreateInfo litHdrFramebufferCreateInfo{};
		litHdrFramebufferCreateInfo.debugName = framebufferName.c_str();
		litHdrFramebufferCreateInfo.width = width;
		litHdrFramebufferCreateInfo.height = height;
		litHdrFramebufferCreateInfo.renderTargetLists = &imageSet.litHdrRenderTarget;
		litHdrFramebufferCreateInfo.numRenderTargetLists = 1;
		litHdrFramebufferCreateInfo.depthTarget = imageSet.gbufferDepthTarget;
		litHdrFramebufferCreateInfo.renderPass = mainRenderPass;
		imageSet.litHdrFramebuffer = graphicsCore->CreateFramebuffer(litHdrFramebufferCreateInfo);
	}
}

void DeferredRenderer::CreatePipelines() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsPipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.primitiveType = GeometryType::Triangles;
	pipelineCreateInfo.cullMode = CullMode::Back;
	pipelineCreateInfo.scissorX = 0;
	pipelineCreateInfo.scissorY = 0;
	pipelineCreateInfo.vertexBindings = &vertexLightPositionLayout;
	pipelineCreateInfo.vertexBindingsCount = 1;

	std::vector<ShaderStageCreateInfo> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	auto assetManager = EngineCore::GetInstance().assetManager;
	uint8_t shaderBits = static_cast<uint8_t>(ShaderStageBit::Vertex | ShaderStageBit::Fragment);
	 
	{
		if (!assetManager->LoadShaderSet(Uuid("3b3bc2c8-ac88-4fba-b9f9-704f86c1278c"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load ssao shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 2> ssaoLayouts{};
		ssaoLayouts[0] = lightingDescriptorSetLayout;
		ssaoLayouts[1] = ssaoDescriptorSetLayout;

		pipelineCreateInfo.debugName = "SSAO Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = ssaoLayouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(ssaoLayouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = ssaoRenderPass;
		pipelineCreateInfo.width = static_cast<float>(ssaoFramebuffer->GetWidth());
		pipelineCreateInfo.height = static_cast<float>(ssaoFramebuffer->GetHeight());
		pipelineCreateInfo.scissorW = ssaoFramebuffer->GetWidth();
		pipelineCreateInfo.scissorH = ssaoFramebuffer->GetHeight();
		ssaoPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	pipelineCreateInfo.width = static_cast<float>(width);
	pipelineCreateInfo.height = static_cast<float>(height);
	pipelineCreateInfo.scissorW = width;
	pipelineCreateInfo.scissorH = height;

	{
		ShaderStageCreateInfo bloomShaderStageCreateInfo;
		std::vector<char> bloomFileData;

		if (!assetManager->LoadShaderStage(Uuid("8a2475b4-8731-456c-beb7-2d51db7914f9"), ShaderStage::Compute, bloomShaderStageCreateInfo, bloomFileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load bloom compute shader.");
			return;
		}

		ComputePipeline::CreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.debugName = "Bloom Compute Pipeline";
		computePipelineCreateInfo.shaderFileName = bloomShaderStageCreateInfo.fileName;
		computePipelineCreateInfo.shaderContent = bloomFileData.data();
		computePipelineCreateInfo.shaderSize = static_cast<uint32_t>(bloomFileData.size());
		computePipelineCreateInfo.descriptorSetLayouts = &bloomDescriptorSetLayout;
		computePipelineCreateInfo.descriptorSetLayoutCount = 1;
		bloomPipeline = graphicsCore->CreateComputePipeline(computePipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(Uuid("5227a9a2-4a62-4f1b-9906-2b6acbf1b8d3"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load image based lighting shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 3> iblLayouts{};
		iblLayouts[0] = lightingDescriptorSetLayout;
		iblLayouts[1] = ssaoInputDescriptorSetLayout;
		iblLayouts[2] = environmentMapDescriptorSetLayout;

		pipelineCreateInfo.debugName = "Image Based Lighting Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = iblLayouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(iblLayouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		pipelineCreateInfo.blendMode = BlendMode::Additive;
		pipelineCreateInfo.renderPass = mainRenderPass;
		imageBasedLightingPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(Uuid("5537b925-96bc-4e1f-8e2a-d66d6dd9bed1"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, "Could not load point light shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 2> pointLightLayouts{};
		pointLightLayouts[0] = lightingDescriptorSetLayout;
		pointLightLayouts[1] = lightingUBODescriptorSetLayout;

		pipelineCreateInfo.debugName = "Point Light Pipeline";
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
		pointLightPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
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

		pipelineCreateInfo.debugName = "Spot Light Pipeline";
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
		spotLightPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
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

		pipelineCreateInfo.debugName = "Directional Light Pipeline";
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
		directionalLightPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
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

		pipelineCreateInfo.debugName = "Tonemapping Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = &tonemapDescriptorSetLayout;
		pipelineCreateInfo.descriptorSetLayoutCount = 1;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = targetRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = true;
		pipelineCreateInfo.isDepthTestEnabled = true;
		pipelineCreateInfo.isStencilEnabled = true;
		tonemapPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
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
		pipelineCreateInfo.debugName = "Shadow Mapping Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = &shadowMapDescriptorSetLayout;
		pipelineCreateInfo.descriptorSetLayoutCount = 1;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.cullMode = CullMode::None;
		pipelineCreateInfo.renderPass = shadowMapRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = true;
		pipelineCreateInfo.isDepthTestEnabled = true;
		pipelineCreateInfo.isStencilEnabled = false;
		pipelineCreateInfo.hasDynamicScissor = true;
		pipelineCreateInfo.hasDynamicViewport = true;
		pipelineCreateInfo.isDepthBiasEnabled = true;
		shadowMappingPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}
	pipelineCreateInfo.isDepthBiasEnabled = false;
}

void DeferredRenderer::RenderLightsImmediate(entt::registry& registry) {
#if 0
	GRIND_PROFILE_FUNC();
	if (pointLightPipeline == nullptr) {
		return;
	}

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	graphicsCore->BindGraphicsPipeline(pointLightPipeline);
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

void DeferredRenderer::RenderBloom(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	if (bloomPipeline == nullptr) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	currentCommandBuffer->BindComputePipeline(bloomPipeline);
	uint32_t groupCountX = static_cast<uint32_t>(std::ceil(width / 4.0f));
	uint32_t groupCountY = static_cast<uint32_t>(std::ceil(height / 4.0f));
	uint32_t descriptorSetIndex = 0;

	{
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[1], true);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}

	uint32_t mipWidth = static_cast<uint32_t>(width);
	uint32_t mipHeight = static_cast<uint32_t>(height);

	std::vector<uint32_t> mipWidths(bloomMipLevelCount);
	std::vector<uint32_t> mipHeights(bloomMipLevelCount);
	mipWidths[0] = mipWidth;
	mipHeights[0] = mipHeight;

	for (size_t i = 1; i < bloomMipLevelCount - 1; ++i) {
		mipWidth /= 2;
		mipHeight /= 2;
		mipWidths[i] = mipWidth;
		mipHeights[i] = mipHeight;

		groupCountX = static_cast<uint32_t>(std::ceil(mipWidth / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeight / 4.0f));

		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[i], false);
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[i + 1], true);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}

	{
		groupCountX = static_cast<uint32_t>(std::ceil(mipWidths[bloomMipLevelCount  - 2] / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeights[bloomMipLevelCount - 2] / 4.0f));

		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomMipLevelCount * 2 - 1], true);
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomMipLevelCount - 1], false);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}

	for (size_t i = bloomMipLevelCount - 3; i != SIZE_MAX; --i) {
		mipWidth = mipWidths[i];
		mipHeight = mipHeights[i];
		groupCountX = static_cast<uint32_t>(std::ceil(mipWidth / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeight / 4.0f));

		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomMipLevelCount + i + 1], true);
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomMipLevelCount + i + 2], false);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}

	currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomMipLevelCount + 1], false);
}

void DeferredRenderer::RenderLightsCommandBuffer(
	uint32_t imageIndex,
	GraphicsAPI::CommandBuffer* currentCommandBuffer,
	entt::registry& registry
) {
	GRIND_PROFILE_FUNC();

	EngineCore& engineCore = EngineCore::GetInstance();
	auto graphicsCore = engineCore.GetGraphicsCore();
	auto& imageSet = deferredRendererImageSets[imageIndex];
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;
	currentCommandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	currentCommandBuffer->BindIndexBuffer(indexBuffer);

	const glm::mat4 bias = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);

	if (imageBasedLightingPipeline != nullptr) {
		// Image Based Lighting Lights
		currentCommandBuffer->BindGraphicsPipeline(imageBasedLightingPipeline);

		auto view = registry.view<const EnvironmentMapComponent>();

		bool hasEnvMap = false;
		view.each([&](const EnvironmentMapComponent& environmentMapComponent) {
			if (currentEnvironmentMapUuid == environmentMapComponent.specularTexture.uuid) {
				return;
			}

			currentEnvironmentMapUuid = environmentMapComponent.specularTexture.uuid;
			Grindstone::TextureAsset* texAsset = environmentMapComponent.specularTexture.Get();
			if (texAsset != nullptr) {
				Texture* tex = texAsset->texture;
				hasEnvMap = true;

				GraphicsAPI::DescriptorSet::Binding binding{};
				binding.bindingIndex = 0;
				binding.bindingType = GraphicsAPI::BindingType::Texture;
				binding.count = 1;
				binding.itemPtr = tex;

				environmentMapDescriptorSet->ChangeBindings(&binding, 1);
			}
		});

		if (hasEnvMap) {
			std::array<GraphicsAPI::DescriptorSet*, 3> iblDescriptors{};
			iblDescriptors[0] = imageSet.lightingDescriptorSet;
			iblDescriptors[1] = ssaoInputDescriptorSet;
			iblDescriptors[2] = environmentMapDescriptorSet;
			currentCommandBuffer->BindGraphicsDescriptorSet(imageBasedLightingPipeline, iblDescriptors.data(), static_cast<uint32_t>(iblDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		}
	}

	if (pointLightPipeline != nullptr) {
		// Point Lights
		currentCommandBuffer->BindGraphicsPipeline(pointLightPipeline);

		std::array<GraphicsAPI::DescriptorSet*, 2> pointLightDescriptors{};
		pointLightDescriptors[0] = imageSet.lightingDescriptorSet;

		auto view = registry.view<const entt::entity, PointLightComponent>();
		view.each([&](const entt::entity entityHandle, PointLightComponent& pointLightComponent) {
			const ECS::Entity entity(entityHandle, scene);

			PointLightComponent::UniformStruct lightmapStruct{
				pointLightComponent.color,
				pointLightComponent.attenuationRadius,
				entity.GetWorldPosition(),
				pointLightComponent.intensity
			};

			pointLightDescriptors[1] = pointLightComponent.descriptorSet;
			pointLightComponent.uniformBufferObject->UpdateBuffer(&lightmapStruct);
			currentCommandBuffer->BindGraphicsDescriptorSet(pointLightPipeline, pointLightDescriptors.data(), static_cast<uint32_t>(pointLightDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}

	if (spotLightPipeline != nullptr) {
		// Spot Lights
		currentCommandBuffer->BindGraphicsPipeline(spotLightPipeline);

		std::array<GraphicsAPI::DescriptorSet*, 2> spotLightDescriptors{};
		spotLightDescriptors[0] = imageSet.lightingDescriptorSet;

		auto view = registry.view<const entt::entity, SpotLightComponent>();
		view.each([&](const entt::entity entityHandle, SpotLightComponent& spotLightComponent) {
			const ECS::Entity entity(entityHandle, scene);

			SpotLightComponent::UniformStruct lightStruct {
				bias * spotLightComponent.shadowMatrix,
				spotLightComponent.color,
				spotLightComponent.attenuationRadius,
				entity.GetWorldPosition(),
				spotLightComponent.intensity,
				entity.GetWorldForward(),
				glm::cos(glm::radians(spotLightComponent.innerAngle)),
				glm::cos(glm::radians(spotLightComponent.outerAngle)),
				spotLightComponent.shadowResolution
			};

			spotLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

			spotLightDescriptors[1] = spotLightComponent.descriptorSet;
			currentCommandBuffer->BindGraphicsDescriptorSet(spotLightPipeline, spotLightDescriptors.data(), static_cast<uint32_t>(spotLightDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}

	if (directionalLightPipeline != nullptr) {
		// Directional Lights
		currentCommandBuffer->BindGraphicsPipeline(directionalLightPipeline);

		std::array<GraphicsAPI::DescriptorSet*, 2> directionalLightDescriptors{};
		directionalLightDescriptors[0] = imageSet.lightingDescriptorSet;

		auto view = registry.view<const entt::entity, const TransformComponent, DirectionalLightComponent>();
		view.each([&](const entt::entity entityHandle, const TransformComponent& transformComponent, DirectionalLightComponent& directionalLightComponent) {
			const ECS::Entity entity(entityHandle, scene);

			DirectionalLightComponent::UniformStruct lightStruct{
				bias * directionalLightComponent.shadowMatrix,
				directionalLightComponent.color,
				directionalLightComponent.sourceRadius,
				entity.GetWorldForward(),
				directionalLightComponent.intensity,
				directionalLightComponent.shadowResolution
			};

			directionalLightComponent.uniformBufferObject->UpdateBuffer(&lightStruct);

			directionalLightDescriptors[1] = directionalLightComponent.descriptorSet;
			currentCommandBuffer->BindGraphicsDescriptorSet(directionalLightPipeline, directionalLightDescriptors.data(), static_cast<uint32_t>(directionalLightDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		});
	}
}

void DeferredRenderer::RenderSsao(uint32_t imageIndex, GraphicsAPI::CommandBuffer* commandBuffer) {
	if (ssaoPipeline == nullptr) {
		return;
	}

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	auto& imageSet = deferredRendererImageSets[imageIndex];

	ClearColorValue clearColorAttachment = { 16.0f, 16.0f, 16.0f, 16.0f };
	ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = false;

	commandBuffer->BindRenderPass(
		ssaoRenderPass,
		ssaoFramebuffer,
		ssaoFramebuffer->GetWidth(),
		ssaoFramebuffer->GetHeight(),
		&clearColorAttachment,
		1,
		clearDepthStencil
	);

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	std::array<GraphicsAPI::DescriptorSet*, 2> ssaoDescriptors{};
	ssaoDescriptors[0] = imageSet.lightingDescriptorSet;
	ssaoDescriptors[1] = ssaoDescriptorSet;

	commandBuffer->BindGraphicsPipeline(ssaoPipeline);
	commandBuffer->BindGraphicsDescriptorSet(ssaoPipeline, ssaoDescriptors.data(), static_cast<uint32_t>(ssaoDescriptors.size()));
	commandBuffer->DrawIndices(0, 6, 1, 0);
	commandBuffer->UnbindRenderPass();
}

void DeferredRenderer::RenderShadowMaps(CommandBuffer* commandBuffer, entt::registry& registry) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	AssetRendererManager* assetManager = engineCore.assetRendererManager;
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;

	ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;

	/* TODO: Finish with Point Light Shadows eventually
	{
		auto view = registry.view<const TransformComponent, PointLightComponent>();
		view.each([&](const TransformComponent& transformComponent, PointLightComponent& pointLightComponent) {
			float farDist = pointLightComponent.attenuationRadius;

			const glm::vec3 forwardVector = transformComponent.GetForward();
			const glm::vec3 pos = transformComponent.position;

			const auto viewMatrix = glm::lookAt(
				pos,
				pos + forwardVector,
				transformComponent.GetUp()
			);

			constexpr float fov = 90.0f;
			auto projectionMatrix = glm::perspective(
				fov,
				1.0f,
				0.1f,
				farDist
			);

			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			glm::mat4 shadowPass = projectionMatrix * viewMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			// pointLightComponent.shadowMatrix = projectionMatrix * viewMatrix * glm::mat4(1.0f);

			uint32_t resolution = static_cast<uint32_t>(pointLightComponent.shadowResolution);

			pointLightComponent.shadowMapUniformBufferObject->UpdateBuffer(&shadowPass);

			commandBuffer->BindRenderPass(
				pointLightComponent.renderPass,
				pointLightComponent.framebuffer,
				resolution,
				resolution,
				nullptr,
				0,
				clearDepthStencil
			);

			commandBuffer->BindGraphicsPipeline(shadowMappingPipeline);

			float resF = static_cast<float>(resolution);
			commandBuffer->SetViewport(0.0f, 0.0f, resF, resF);
			commandBuffer->SetScissor(0, 0, resolution, resolution);

			commandBuffer->BindGraphicsDescriptorSet(shadowMappingPipeline, &pointLightComponent.shadowMapDescriptorSet, 1);
			assetManager->RenderShadowMap(
				commandBuffer,
				spotLightComponent.shadowMapDescriptorSet,
				registry,
				transformComponent.position
			);

			commandBuffer->UnbindRenderPass();
		});
	}
	*/

	{
		auto view = registry.view<const entt::entity, SpotLightComponent>();
		view.each([&](const entt::entity entityHandle, SpotLightComponent& spotLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);

			float fov = glm::radians(spotLightComponent.outerAngle * 2.0f);
			float farDist = spotLightComponent.attenuationRadius;

			const glm::vec3 forwardVector = entity.GetWorldForward();
			const glm::vec3 pos = entity.GetWorldPosition();

			const auto viewMatrix = glm::lookAt(
				pos,
				pos + forwardVector,
				entity.GetWorldUp()
			);

			auto projectionMatrix = glm::perspective(
				fov,
				1.0f,
				0.1f,
				farDist
			);

			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			glm::mat4 shadowPass = projectionMatrix * viewMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			spotLightComponent.shadowMatrix = projectionMatrix * viewMatrix * glm::mat4(1.0f);

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

			commandBuffer->BindGraphicsPipeline(shadowMappingPipeline);

			float resF = static_cast<float>(resolution);
			commandBuffer->SetViewport(0.0f, 0.0f, resF, resF);
			commandBuffer->SetScissor(0, 0, resolution, resolution);

			commandBuffer->BindGraphicsDescriptorSet(shadowMappingPipeline, &spotLightComponent.shadowMapDescriptorSet, 1);
			assetManager->RenderShadowMap(
				commandBuffer,
				spotLightComponent.shadowMapDescriptorSet,
				registry,
				pos
			);

			commandBuffer->UnbindRenderPass();
		});
	}

	{
		auto view = registry.view<const entt::entity, DirectionalLightComponent>();
		view.each([&](const entt::entity entityHandle, DirectionalLightComponent& directionalLightComponent) {
			const ECS::Entity entity = ECS::Entity(entityHandle, scene);

			const float shadowHalfSize = 40.0f;
			glm::mat4 projectionMatrix = glm::ortho<float>(-shadowHalfSize, shadowHalfSize, -shadowHalfSize, shadowHalfSize, 0, 160.0f);
			graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

			Math::Float3 forward = entity.GetWorldForward();

			glm::vec3 lightPos = forward * -100.0f;
			glm::mat4 viewMatrix = glm::lookAt(
				lightPos,
				glm::vec3(0, 0, 0),
				entity.GetWorldUp()
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

			commandBuffer->BindGraphicsPipeline(shadowMappingPipeline);

			float resF = static_cast<float>(resolution);
			commandBuffer->SetViewport(0.0f, 0.0f, resF, resF);
			commandBuffer->SetScissor(0, 0, resolution, resolution);

			commandBuffer->BindGraphicsDescriptorSet(shadowMappingPipeline, &directionalLightComponent.shadowMapDescriptorSet, 1);
			assetManager->RenderShadowMap(
				commandBuffer,
				directionalLightComponent.shadowMapDescriptorSet,
				registry,
				lightPos
			);

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

	RenderBloom(imageSet, currentCommandBuffer);

	ClearColorValue clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = false;
	auto renderPass = framebuffer->GetRenderPass();
	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, framebuffer->GetWidth(), framebuffer->GetHeight(), &clearColor, 1, clearDepthStencil);

	if (tonemapPipeline != nullptr) {
		currentCommandBuffer->BindGraphicsDescriptorSet(tonemapPipeline, &imageSet.tonemapDescriptorSet, 1);
		currentCommandBuffer->BindVertexBuffers(&vertexBuffer, 1);
		currentCommandBuffer->BindIndexBuffer(indexBuffer);

		{
			// Tonemapping
			currentCommandBuffer->BindGraphicsPipeline(tonemapPipeline);
			currentCommandBuffer->DrawIndices(0, 6, 1, 0);
		}
	}

	currentCommandBuffer->UnbindRenderPass();
}

void DeferredRenderer::Render(
	GraphicsAPI::CommandBuffer* commandBuffer,
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
			commandBuffer,
			registry,
			projectionMatrix,
			viewMatrix,
			eyePos,
			outputFramebuffer
		);
	}
}

void DeferredRenderer::RenderCommandBuffer(
	GraphicsAPI::CommandBuffer* commandBuffer,
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::Framebuffer* outputFramebuffer
) {
	Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::AssetRendererManager* assetManager = engineCore.assetRendererManager;
	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

	assetManager->CacheRenderTasksAndFrustumCull(eyePos, registry);
	assetManager->SortQueues();

	graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

	uint32_t imageIndex = wgb->GetCurrentImageIndex();
	auto& imageSet = deferredRendererImageSets[imageIndex];

	EngineUboStruct engineUboStruct{};
	engineUboStruct.proj = projectionMatrix;
	engineUboStruct.view = viewMatrix;
	engineUboStruct.eyePos = eyePos;
	imageSet.globalUniformBufferObject->UpdateBuffer(&engineUboStruct);

	RenderShadowMaps(commandBuffer, registry);
	assetManager->SetEngineDescriptorSet(imageSet.engineDescriptorSet);

	uint32_t width = imageSet.gbuffer->GetWidth();
	uint32_t height = imageSet.gbuffer->GetHeight();

	{
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

		commandBuffer->BindRenderPass(gbufferRenderPass, imageSet.gbuffer, width, height, clearColors, 4, clearDepthStencil);
	}

	commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	commandBuffer->SetScissor(0, 0, width, height);

	assetManager->RenderQueue(commandBuffer, "Opaque");
	commandBuffer->UnbindRenderPass();

	RenderSsao(imageIndex, commandBuffer);

	{
		ClearColorValue clearColor = { 0.0f, 0.0f, 0.0f, 0.f };
		ClearDepthStencil clearDepthStencil;
		clearDepthStencil.depth = 1.0f;
		clearDepthStencil.stencil = 0;
		clearDepthStencil.hasDepthStencilAttachment = true;
		commandBuffer->BindRenderPass(mainRenderPass, imageSet.litHdrFramebuffer, imageSet.litHdrFramebuffer->GetWidth(), imageSet.litHdrFramebuffer->GetHeight(), &clearColor, 1, clearDepthStencil);
	}

	RenderLightsCommandBuffer(imageIndex, commandBuffer, registry);
	assetManager->RenderQueue(commandBuffer, "Unlit");
	assetManager->RenderQueue(commandBuffer, "Skybox");
	assetManager->RenderQueue(commandBuffer, "Transparent");
	commandBuffer->UnbindRenderPass();

	PostProcessCommandBuffer(imageIndex, outputFramebuffer, commandBuffer);
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
