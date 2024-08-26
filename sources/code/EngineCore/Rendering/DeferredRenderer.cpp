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
#include <Common/HashedString.hpp>

#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Shaders/ShaderImporter.hpp>
#include <EngineCore/Assets/Materials/MaterialImporter.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/CoreComponents/EnvironmentMap/EnvironmentMapComponent.hpp>
#include <EngineCore/CoreComponents/Lights/PointLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/SpotLightComponent.hpp>
#include <EngineCore/CoreComponents/Lights/DirectionalLightComponent.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/Profiling.hpp>

#include "DeferredRenderer.hpp"
#include <EngineCore/Logger.hpp>

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

std::array<Grindstone::BaseRenderer::RenderMode, static_cast<uint16_t>(DeferredRenderer::DeferredRenderMode::Count)> DeferredRenderer::renderModes = {
	Grindstone::BaseRenderer::RenderMode{ "Default" },
	Grindstone::BaseRenderer::RenderMode{ "World Position" },
	Grindstone::BaseRenderer::RenderMode{ "World Position (Modulus)" },
	Grindstone::BaseRenderer::RenderMode{ "View Position" },
	Grindstone::BaseRenderer::RenderMode{ "View Position (Modulus)" },
	Grindstone::BaseRenderer::RenderMode{ "Depth" },
	Grindstone::BaseRenderer::RenderMode{ "Depth (Modulus)" },
	Grindstone::BaseRenderer::RenderMode{ "Normals" },
	Grindstone::BaseRenderer::RenderMode{ "View Normals" },
	Grindstone::BaseRenderer::RenderMode{ "Albedo" },
	Grindstone::BaseRenderer::RenderMode{ "Specular" },
	Grindstone::BaseRenderer::RenderMode{ "Roughness" },
	Grindstone::BaseRenderer::RenderMode{ "Ambient Occlusion" }
};

GraphicsAPI::RenderPass* DeferredRenderer::gbufferRenderPass = nullptr;
GraphicsAPI::RenderPass* DeferredRenderer::mainRenderPass = nullptr;

const size_t MAX_BLOOM_MIPS = 40u;
const DepthFormat depthFormat = DepthFormat::D24;
const bool shouldFastResize = true;
GraphicsAPI::ColorFormat ambientOcclusionFormat = GraphicsAPI::ColorFormat::R8;;

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
	float buffer;
	glm::vec2 framebufferResolution;
	glm::vec2 renderResolution;
	glm::vec2 renderScale;
	float time;
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
	glm::vec2 inReciprocalImgSize;
	glm::vec2 outReciprocalImgSize;
	glm::vec4 thresholdFilter; // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
	BloomStage stage;
	float levelOfDetail;
	float filterRadius;
};

static size_t CalculateBloomLevels(uint32_t width, uint32_t height) {
	float minDimension = static_cast<float>(glm::min(width, height));
	float logDimension = glm::log2(minDimension);
	return glm::min(static_cast<size_t>(logDimension) - 3, MAX_BLOOM_MIPS);
}

DeferredRenderer::DeferredRenderer(GraphicsAPI::RenderPass* targetRenderPass) : targetRenderPass(targetRenderPass) {
	if (shouldFastResize) {
		Display& display = EngineCore::GetInstance().displayManager->GetMainDisplay();
		framebufferWidth = display.width;
		framebufferHeight = display.height;
		renderWidth = display.width;
		renderHeight = display.height;
	}

	renderMode = DeferredRenderMode::Default;

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	auto wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();
	uint32_t maxFramesInFlight = wgb->GetMaxFramesInFlight();
	deferredRendererImageSets.resize(maxFramesInFlight);

	RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Shadow Map Render Pass";
	renderPassCreateInfo.colorAttachments = nullptr;
	renderPassCreateInfo.colorAttachmentCount = 0;
	renderPassCreateInfo.depthFormat = DepthFormat::D32;
	shadowMapRenderPass = graphicsCore->CreateRenderPass(renderPassCreateInfo);

	bloomStoredMipLevelCount = bloomMipLevelCount = CalculateBloomLevels(renderWidth, renderHeight);
	bloomFirstUpsampleIndex = bloomStoredMipLevelCount - 1;

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
	CreateDepthOfFieldResources();
	CreateSSRResources();
	CreateBloomResources();
	CreateBloomUniformBuffers();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		GraphicsAPI::UniformBuffer::CreateInfo postProcessingUboCreateInfo{};
		postProcessingUboCreateInfo.debugName = "Post Processing UBO";
		postProcessingUboCreateInfo.isDynamic = false;
		postProcessingUboCreateInfo.size = sizeof(PostProcessUbo);
		GraphicsAPI::UniformBuffer* postProcessingUbo =
			graphicsCore->CreateUniformBuffer(postProcessingUboCreateInfo);
		postProcessingUbo->UpdateBuffer(&postProcessUboData);
		deferredRendererImageSets[i].tonemapPostProcessingUniformBufferObject = postProcessingUbo;

		CreateDepthOfFieldRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateSsrRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateBloomRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateDescriptorSets(deferredRendererImageSets[i]);
	}

	CreatePipelines();
}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	CleanupPipelines();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		DeferredRendererImageSet& imageSet = deferredRendererImageSets[i];

		graphicsCore->DeleteFramebuffer(imageSet.lightingFramebuffer);
		graphicsCore->DeleteRenderTarget(imageSet.ssrRenderTarget);

		graphicsCore->DeleteUniformBuffer(imageSet.debugUniformBufferObject);
		graphicsCore->DeleteUniformBuffer(imageSet.tonemapPostProcessingUniformBufferObject);

		graphicsCore->DeleteDescriptorSet(imageSet.ssrDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.debugDescriptorSet);

		graphicsCore->DeleteDescriptorSet(imageSet.dofSourceDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.dofNearBlurDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.dofFarBlurDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.dofCombineDescriptorSet);

		graphicsCore->DeleteRenderTarget(imageSet.nearDofRenderTarget);
		graphicsCore->DeleteRenderTarget(imageSet.farDofRenderTarget);
		graphicsCore->DeleteRenderTarget(imageSet.nearBlurredDofRenderTarget);
		graphicsCore->DeleteRenderTarget(imageSet.farBlurredDofRenderTarget);

		graphicsCore->DeleteFramebuffer(imageSet.dofSeparationFramebuffer);
		graphicsCore->DeleteFramebuffer(imageSet.dofNearBlurFramebuffer);
		graphicsCore->DeleteFramebuffer(imageSet.dofFarBlurFramebuffer);
		graphicsCore->DeleteFramebuffer(imageSet.dofCombinationFramebuffer);

		for (GraphicsAPI::RenderTarget* bloomRt : imageSet.bloomRenderTargets) {
			graphicsCore->DeleteRenderTarget(bloomRt);
		}

		for (GraphicsAPI::DescriptorSet* bloomDs : imageSet.bloomDescriptorSets) {
			graphicsCore->DeleteDescriptorSet(bloomDs);
		}

		graphicsCore->DeleteUniformBuffer(imageSet.globalUniformBufferObject);

		graphicsCore->DeleteFramebuffer(imageSet.gbuffer);
		graphicsCore->DeleteRenderTarget(imageSet.gbufferAlbedoRenderTarget);
		graphicsCore->DeleteRenderTarget(imageSet.gbufferNormalRenderTarget);
		graphicsCore->DeleteRenderTarget(imageSet.gbufferSpecularRoughnessRenderTarget);

		graphicsCore->DeleteDepthTarget(imageSet.gbufferDepthTarget);
		graphicsCore->DeleteFramebuffer(imageSet.litHdrFramebuffer);
		graphicsCore->DeleteRenderTarget(imageSet.litHdrRenderTarget);

		graphicsCore->DeleteDescriptorSet(imageSet.engineDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.tonemapDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.lightingDescriptorSet);

		graphicsCore->DeleteRenderTarget(imageSet.ambientOcclusionRenderTarget);
		graphicsCore->DeleteFramebuffer(imageSet.ambientOcclusionFramebuffer);
		graphicsCore->DeleteDescriptorSet(imageSet.ambientOcclusionDescriptorSet);
	}

	for (GraphicsAPI::UniformBuffer* bloomUb : bloomUniformBuffers) {
		graphicsCore->DeleteUniformBuffer(bloomUb);
	}

	graphicsCore->DeleteDescriptorSetLayout(engineDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(tonemapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingDescriptorSetLayout);

	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);

	graphicsCore->DeleteUniformBuffer(ssaoUniformBuffer);
	graphicsCore->DeleteTexture(ssaoNoiseTexture);
	graphicsCore->DeleteDescriptorSet(ssaoInputDescriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(ssaoInputDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(ambientOcclusionDescriptorSetLayout);

	graphicsCore->DeleteRenderPass(dofSeparationRenderPass);
	graphicsCore->DeleteRenderPass(dofBlurAndCombinationRenderPass);
	graphicsCore->DeleteRenderPass(shadowMapRenderPass);
	graphicsCore->DeleteRenderPass(targetRenderPass);
	graphicsCore->DeleteRenderPass(lightingRenderPass);
	graphicsCore->DeleteRenderPass(forwardLitRenderPass);
	graphicsCore->DeleteRenderPass(ssaoRenderPass);

	graphicsCore->DeleteDescriptorSetLayout(environmentMapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(bloomDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(ssrDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(debugDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingUBODescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(shadowMappedLightDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingWithShadowUBODescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(shadowMapDescriptorSetLayout);

	graphicsCore->DeleteDescriptorSet(environmentMapDescriptorSet);
	graphicsCore->DeleteDescriptorSet(shadowMapDescriptorSet);

	graphicsCore->DeleteVertexBuffer(vertexBuffer);
	graphicsCore->DeleteIndexBuffer(indexBuffer);
	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);

	graphicsCore->DeleteDescriptorSetLayout(dofSourceDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(dofBlurDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(dofCombinationDescriptorSetLayout);
}

void DeferredRenderer::CreateDepthOfFieldRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	if (imageSet.nearDofRenderTarget != nullptr) {
		graphicsCore->DeleteRenderTarget(imageSet.nearDofRenderTarget);
	}

	if (imageSet.farDofRenderTarget != nullptr) {
		graphicsCore->DeleteRenderTarget(imageSet.farDofRenderTarget);
	}

	if (imageSet.nearBlurredDofRenderTarget != nullptr) {
		graphicsCore->DeleteRenderTarget(imageSet.nearBlurredDofRenderTarget);
	}

	if (imageSet.farBlurredDofRenderTarget != nullptr) {
		graphicsCore->DeleteRenderTarget(imageSet.farBlurredDofRenderTarget);
	}

	if (imageSet.dofSeparationFramebuffer != nullptr) {
		graphicsCore->DeleteFramebuffer(imageSet.dofSeparationFramebuffer);
	}

	if (imageSet.dofNearBlurFramebuffer != nullptr) {
		graphicsCore->DeleteFramebuffer(imageSet.dofNearBlurFramebuffer);
	}

	if (imageSet.dofFarBlurFramebuffer != nullptr) {
		graphicsCore->DeleteFramebuffer(imageSet.dofFarBlurFramebuffer);
	}

	if (imageSet.dofCombinationFramebuffer != nullptr) {
		graphicsCore->DeleteFramebuffer(imageSet.dofCombinationFramebuffer);
	}

	if (imageSet.dofSourceDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofSourceDescriptorSet);
	}

	if (imageSet.dofNearBlurDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofNearBlurDescriptorSet);
	}

	if (imageSet.dofFarBlurDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofFarBlurDescriptorSet);
	}

	if (imageSet.dofCombineDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.dofCombineDescriptorSet);
	}

	{
		GraphicsAPI::RenderTarget::CreateInfo nearRTCreateInfo = { GraphicsAPI::ColorFormat::RGBA16, framebufferWidth / 2, framebufferHeight / 2, true, false, "Near DOF Render Target" };
		imageSet.nearDofRenderTarget = graphicsCore->CreateRenderTarget(nearRTCreateInfo);
	}

	{
		GraphicsAPI::RenderTarget::CreateInfo farRTCreateInfo = { GraphicsAPI::ColorFormat::RGBA16, framebufferWidth / 2, framebufferHeight / 2, true, false, "Far DOF Render Target" };
		imageSet.farDofRenderTarget = graphicsCore->CreateRenderTarget(farRTCreateInfo);
	}

	{
		GraphicsAPI::RenderTarget::CreateInfo nearBlurredRTCreateInfo = { GraphicsAPI::ColorFormat::RGBA16, framebufferWidth / 4, framebufferHeight / 4, true, false, "Near Blurred DOF Render Target" };
		imageSet.nearBlurredDofRenderTarget = graphicsCore->CreateRenderTarget(nearBlurredRTCreateInfo);
	}

	{
		GraphicsAPI::RenderTarget::CreateInfo farBlurredRTCreateInfo = { GraphicsAPI::ColorFormat::RGBA16, framebufferWidth / 4, framebufferHeight / 4, true, false, "Far Blurred DOF Render Target" };
		imageSet.farBlurredDofRenderTarget = graphicsCore->CreateRenderTarget(farBlurredRTCreateInfo);
	}

	{
		std::array<RenderTarget*, 2> renderTargets = {
			imageSet.nearDofRenderTarget,
			imageSet.farDofRenderTarget
		};

		GraphicsAPI::Framebuffer::CreateInfo dofSeparationFramebufferCreateInfo{};
		dofSeparationFramebufferCreateInfo.debugName = "Depth of Field Separation Framebuffer";
		dofSeparationFramebufferCreateInfo.depthTarget = nullptr;
		dofSeparationFramebufferCreateInfo.width = framebufferWidth / 2;
		dofSeparationFramebufferCreateInfo.height = framebufferHeight / 2;
		dofSeparationFramebufferCreateInfo.isCubemap = false;
		dofSeparationFramebufferCreateInfo.renderPass = dofSeparationRenderPass;
		dofSeparationFramebufferCreateInfo.renderTargetLists = renderTargets.data();
		dofSeparationFramebufferCreateInfo.numRenderTargetLists = static_cast<uint32_t>(renderTargets.size());
		imageSet.dofSeparationFramebuffer = graphicsCore->CreateFramebuffer(dofSeparationFramebufferCreateInfo);
	}

	{
		GraphicsAPI::Framebuffer::CreateInfo dofSeparationFramebufferCreateInfo{};
		dofSeparationFramebufferCreateInfo.debugName = "Depth of Field Near Framebuffer";
		dofSeparationFramebufferCreateInfo.depthTarget = nullptr;
		dofSeparationFramebufferCreateInfo.width = framebufferWidth / 4;
		dofSeparationFramebufferCreateInfo.height = framebufferHeight / 4;
		dofSeparationFramebufferCreateInfo.isCubemap = false;
		dofSeparationFramebufferCreateInfo.renderPass = dofBlurAndCombinationRenderPass;
		dofSeparationFramebufferCreateInfo.renderTargetLists = &imageSet.nearBlurredDofRenderTarget;
		dofSeparationFramebufferCreateInfo.numRenderTargetLists = 1;
		imageSet.dofNearBlurFramebuffer = graphicsCore->CreateFramebuffer(dofSeparationFramebufferCreateInfo);
	}

	{
		GraphicsAPI::Framebuffer::CreateInfo dofSeparationFramebufferCreateInfo{};
		dofSeparationFramebufferCreateInfo.debugName = "Depth of Field Far Framebuffer";
		dofSeparationFramebufferCreateInfo.depthTarget = nullptr;
		dofSeparationFramebufferCreateInfo.width = framebufferWidth / 4;
		dofSeparationFramebufferCreateInfo.height = framebufferHeight / 4;
		dofSeparationFramebufferCreateInfo.isCubemap = false;
		dofSeparationFramebufferCreateInfo.renderPass = dofBlurAndCombinationRenderPass;
		dofSeparationFramebufferCreateInfo.renderTargetLists = &imageSet.farBlurredDofRenderTarget;
		dofSeparationFramebufferCreateInfo.numRenderTargetLists = 1;
		imageSet.dofFarBlurFramebuffer = graphicsCore->CreateFramebuffer(dofSeparationFramebufferCreateInfo);
	}

	{
		GraphicsAPI::Framebuffer::CreateInfo dofSeparationFramebufferCreateInfo{};
		dofSeparationFramebufferCreateInfo.debugName = "Depth of Field Combination Framebuffer";
		dofSeparationFramebufferCreateInfo.depthTarget = nullptr;
		dofSeparationFramebufferCreateInfo.width = framebufferWidth;
		dofSeparationFramebufferCreateInfo.height = framebufferHeight;
		dofSeparationFramebufferCreateInfo.isCubemap = false;
		dofSeparationFramebufferCreateInfo.renderPass = dofBlurAndCombinationRenderPass;
		dofSeparationFramebufferCreateInfo.renderTargetLists = &imageSet.litHdrRenderTarget;
		dofSeparationFramebufferCreateInfo.numRenderTargetLists = 1;
		imageSet.dofCombinationFramebuffer = graphicsCore->CreateFramebuffer(dofSeparationFramebufferCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> sourceDofDescriptorBindings = {
			imageSet.gbufferDepthTarget,
			imageSet.litHdrRenderTarget
		};

		GraphicsAPI::DescriptorSet::CreateInfo dofSourceDescriptorSetCreateInfo{};
		dofSourceDescriptorSetCreateInfo.layout = dofSourceDescriptorSetLayout;
		dofSourceDescriptorSetCreateInfo.debugName = "Depth of Field Source Descriptor";
		dofSourceDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(sourceDofDescriptorBindings.size());
		dofSourceDescriptorSetCreateInfo.bindings = sourceDofDescriptorBindings.data();
		imageSet.dofSourceDescriptorSet = graphicsCore->CreateDescriptorSet(dofSourceDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSet::Binding nearDofDescriptorBinding = { imageSet.nearDofRenderTarget };

		GraphicsAPI::DescriptorSet::CreateInfo dofBlurNearDescriptorSetCreateInfo{};
		dofBlurNearDescriptorSetCreateInfo.layout = dofBlurDescriptorSetLayout;
		dofBlurNearDescriptorSetCreateInfo.debugName = "Depth of Field Blur Near Descriptor";
		dofBlurNearDescriptorSetCreateInfo.bindingCount = 1u;
		dofBlurNearDescriptorSetCreateInfo.bindings = &nearDofDescriptorBinding;
		imageSet.dofNearBlurDescriptorSet = graphicsCore->CreateDescriptorSet(dofBlurNearDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSet::Binding farDofDescriptorBinding = { imageSet.farDofRenderTarget };

		GraphicsAPI::DescriptorSet::CreateInfo dofBlurFarDescriptorSetCreateInfo{};
		dofBlurFarDescriptorSetCreateInfo.layout = dofBlurDescriptorSetLayout;
		dofBlurFarDescriptorSetCreateInfo.debugName = "Depth of Field Blur Far Descriptor";
		dofBlurFarDescriptorSetCreateInfo.bindingCount = 1u;
		dofBlurFarDescriptorSetCreateInfo.bindings = &farDofDescriptorBinding;
		imageSet.dofFarBlurDescriptorSet = graphicsCore->CreateDescriptorSet(dofBlurFarDescriptorSetCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> nearAndFarDescriptorBindings = {
			imageSet.nearBlurredDofRenderTarget,
			imageSet.farBlurredDofRenderTarget
		};

		GraphicsAPI::DescriptorSet::CreateInfo dofCombinationDescriptorSetCreateInfo{};
		dofCombinationDescriptorSetCreateInfo.layout = dofCombinationDescriptorSetLayout;
		dofCombinationDescriptorSetCreateInfo.debugName = "Depth of Field Combination Descriptor";
		dofCombinationDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(nearAndFarDescriptorBindings.size());
		dofCombinationDescriptorSetCreateInfo.bindings = nearAndFarDescriptorBindings.data();
		imageSet.dofCombineDescriptorSet = graphicsCore->CreateDescriptorSet(dofCombinationDescriptorSetCreateInfo);
	}
}

void DeferredRenderer::CreateDepthOfFieldResources() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	std::array<RenderPass::AttachmentInfo, 2> dofSepFormats{};
	dofSepFormats[0] = { ColorFormat::RGBA16, true };
	dofSepFormats[1] = { ColorFormat::RGBA16, true };

	RenderPass::CreateInfo dofSepRenderPassCreateInfo{};
	dofSepRenderPassCreateInfo.debugName = "Depth of Field Separation Render Pass";
	dofSepRenderPassCreateInfo.colorAttachments = dofSepFormats.data();
	dofSepRenderPassCreateInfo.colorAttachmentCount = static_cast<uint32_t>(dofSepFormats.size());
	dofSepRenderPassCreateInfo.depthFormat = DepthFormat::None;
	dofSeparationRenderPass = graphicsCore->CreateRenderPass(dofSepRenderPassCreateInfo);

	RenderPass::AttachmentInfo dofBlurAndComboFormat = { ColorFormat::RGBA16, true };

	RenderPass::CreateInfo dofBlurAndComboRenderPassCreateInfo{};
	dofBlurAndComboRenderPassCreateInfo.debugName = "Depth of Field Blur and Combo Render Pass";
	dofBlurAndComboRenderPassCreateInfo.colorAttachments = &dofBlurAndComboFormat;
	dofBlurAndComboRenderPassCreateInfo.colorAttachmentCount = 1;
	dofBlurAndComboRenderPassCreateInfo.depthFormat = DepthFormat::None;
	dofBlurAndCombinationRenderPass = graphicsCore->CreateRenderPass(dofBlurAndComboRenderPassCreateInfo);


	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> sourceDofDescriptorBindings = {
			GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, BindingType::DepthTexture, GraphicsAPI::ShaderStageBit::Fragment }, // Depth
			GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, BindingType::RenderTexture, GraphicsAPI::ShaderStageBit::Fragment } // Lit Scene
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofSourceDescriptorSetLayoutCreateInfo{};
		dofSourceDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Source Descriptor Layout";
		dofSourceDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(sourceDofDescriptorBindings.size());
		dofSourceDescriptorSetLayoutCreateInfo.bindings = sourceDofDescriptorBindings.data();
		dofSourceDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(dofSourceDescriptorSetLayoutCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding farDofDescriptorBinding
			{ 0, 1, BindingType::RenderTexture, GraphicsAPI::ShaderStageBit::Fragment };

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofBlurFarDescriptorSetLayoutCreateInfo{};
		dofBlurFarDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Blur Far Descriptor Layout";
		dofBlurFarDescriptorSetLayoutCreateInfo.bindingCount = 1u;
		dofBlurFarDescriptorSetLayoutCreateInfo.bindings = &farDofDescriptorBinding;
		dofBlurDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(dofBlurFarDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> nearAndFarDescriptorBindings = {
			GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, BindingType::RenderTexture, GraphicsAPI::ShaderStageBit::Fragment }, // Near RT
			GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, BindingType::RenderTexture, GraphicsAPI::ShaderStageBit::Fragment } // Far RT
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofCombinationDescriptorSetLayoutCreateInfo{};
		dofCombinationDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Combination Descriptor Layout";
		dofCombinationDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(nearAndFarDescriptorBindings.size());
		dofCombinationDescriptorSetLayoutCreateInfo.bindings = nearAndFarDescriptorBindings.data();
		dofCombinationDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(dofCombinationDescriptorSetLayoutCreateInfo);
	}
}

void DeferredRenderer::CreateBloomResources() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

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

void DeferredRenderer::CreateSSRResources() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSetLayout::Binding sourceBinding{};
	sourceBinding.count = 1;
	sourceBinding.type = BindingType::RenderTexture;
	sourceBinding.stages = GraphicsAPI::ShaderStageBit::Compute;

	std::array<DescriptorSetLayout::Binding, 6> ssrLayoutBindings{};
	for (size_t i = 0; i < ssrLayoutBindings.size(); ++i) {
		ssrLayoutBindings[i] = sourceBinding;
		ssrLayoutBindings[i].bindingId = static_cast<uint32_t>(i);
	}

	ssrLayoutBindings[0].type = BindingType::UniformBuffer;
	ssrLayoutBindings[1].type = BindingType::RenderTextureStorageImage;
	ssrLayoutBindings[3].type = BindingType::DepthTexture;

	DescriptorSetLayout::CreateInfo ssrDescriptorSetLayoutCreateInfo{};
	ssrDescriptorSetLayoutCreateInfo.debugName = "SSR Descriptor Set Layout";
	ssrDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssrLayoutBindings.size());
	ssrDescriptorSetLayoutCreateInfo.bindings = ssrLayoutBindings.data();
	ssrDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssrDescriptorSetLayoutCreateInfo);
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
		ssaoNoiseCreateInfo.format = GraphicsAPI::ColorFormat::RG8;
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
		ssaoDescriptorSetLayoutCreateInfo.debugName = "SSAO Input Descriptor Set Layout";
		ssaoDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		ssaoDescriptorSetLayoutCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoInputDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssaoDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<DescriptorSet::Binding, 2> ssaoLayoutBindings{
			ssaoUniformBuffer,
			ssaoNoiseTexture
		};

		DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
		engineDescriptorSetCreateInfo.debugName = "SSAO Input Descriptor Set";
		engineDescriptorSetCreateInfo.layout = ssaoInputDescriptorSetLayout;
		engineDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		engineDescriptorSetCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoInputDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::RenderPass::AttachmentInfo attachment{ ambientOcclusionFormat, true };
		GraphicsAPI::RenderPass::CreateInfo ssaoRenderPassCreateInfo{};
		ssaoRenderPassCreateInfo.debugName = "SSAO Renderpass";
		ssaoRenderPassCreateInfo.colorAttachments = &attachment;
		ssaoRenderPassCreateInfo.colorAttachmentCount = 1;
		ssaoRenderPassCreateInfo.depthFormat = DepthFormat::None;
		ssaoRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		ssaoRenderPass = graphicsCore->CreateRenderPass(ssaoRenderPassCreateInfo);
	}
}

void DeferredRenderer::CleanupPipelines() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->DeleteGraphicsPipeline(pointLightPipeline);
	graphicsCore->DeleteGraphicsPipeline(spotLightPipeline);
	graphicsCore->DeleteGraphicsPipeline(directionalLightPipeline);
	graphicsCore->DeleteGraphicsPipeline(shadowMappingPipeline);
	graphicsCore->DeleteGraphicsPipeline(tonemapPipeline);

	graphicsCore->DeleteGraphicsPipeline(ssaoPipeline);
	graphicsCore->DeleteGraphicsPipeline(imageBasedLightingPipeline);
	graphicsCore->DeleteGraphicsPipeline(dofSeparationPipeline);
	graphicsCore->DeleteGraphicsPipeline(dofBlurPipeline);
	graphicsCore->DeleteGraphicsPipeline(dofCombinationPipeline);
	graphicsCore->DeleteGraphicsPipeline(debugPipeline);

	graphicsCore->DeleteComputePipeline(ssrPipeline);
	graphicsCore->DeleteComputePipeline(bloomPipeline);

	pointLightPipeline = nullptr;
	spotLightPipeline = nullptr;
	directionalLightPipeline = nullptr;
	shadowMappingPipeline = nullptr;
	tonemapPipeline = nullptr;
	ssaoPipeline = nullptr;
	imageBasedLightingPipeline = nullptr;
	dofSeparationPipeline = nullptr;
	dofCombinationPipeline = nullptr;
	debugPipeline = nullptr;

	ssrPipeline = nullptr;
	bloomPipeline = nullptr;
}

bool DeferredRenderer::OnWindowResize(Events::BaseEvent* ev) {
	if (ev->GetEventType() == Events::EventType::WindowResize) {
		Events::WindowResizeEvent* winResizeEvent = (Events::WindowResizeEvent*)ev;
		Resize(winResizeEvent->width, winResizeEvent->height);
	}

	return false;
}

void DeferredRenderer::Resize(uint32_t width, uint32_t height) {
	if (width == framebufferWidth && height == framebufferHeight) {
		return;
	}

	this->renderWidth = width;
	this->renderHeight = height;

	uint32_t halfWidth = width / 2u;
	uint32_t halfHeight = height / 2u;

	bloomMipLevelCount = CalculateBloomLevels(width, height);

	if (shouldFastResize && width <= framebufferWidth && height <= framebufferHeight) {
		UpdateBloomUBO();
		for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
			UpdateBloomDescriptorSet(deferredRendererImageSets[i]);
		}

		return;
	}

	if (width > framebufferWidth) {
		framebufferWidth = width;
	}

	if (height > framebufferHeight) {
		framebufferHeight = height;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->WaitUntilIdle();

	bloomStoredMipLevelCount = bloomMipLevelCount;
	bloomFirstUpsampleIndex = bloomStoredMipLevelCount - 1;

	CreateBloomUniformBuffers();
	UpdateBloomUBO();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		DeferredRendererImageSet& imageSet = deferredRendererImageSets[i];

		imageSet.gbufferAlbedoRenderTarget->Resize(width, height);
		imageSet.gbufferNormalRenderTarget->Resize(width, height);
		imageSet.gbufferSpecularRoughnessRenderTarget->Resize(width, height);

		imageSet.gbufferDepthTarget->Resize(width, height);
		imageSet.gbuffer->Resize(width, height);
		imageSet.litHdrRenderTarget->Resize(width, height);
		imageSet.litHdrFramebuffer->Resize(width, height);

		imageSet.ambientOcclusionRenderTarget->Resize(halfWidth, halfHeight);
		imageSet.ambientOcclusionFramebuffer->Resize(halfWidth, halfHeight);

		CreateDepthOfFieldRenderTargetsAndDescriptorSets(imageSet, i);
		CreateSsrRenderTargetsAndDescriptorSets(imageSet, i);
		CreateBloomRenderTargetsAndDescriptorSets(imageSet, i);
		UpdateDescriptorSets(imageSet);
	}
}

void DeferredRenderer::CreateBloomUniformBuffers() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	for (size_t i = 0; i < bloomUniformBuffers.size(); ++i) {
		if (bloomUniformBuffers[i] != nullptr) {
			graphicsCore->DeleteUniformBuffer(bloomUniformBuffers[i]);
		}
	}

	bloomUniformBuffers.resize(bloomStoredMipLevelCount * 2);

	GraphicsAPI::UniformBuffer::CreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.debugName = "Bloom Uniform Buffer";
	uniformBufferCreateInfo.isDynamic = true;
	uniformBufferCreateInfo.size = sizeof(BloomUboStruct);

	for (size_t i = 0; i < bloomUniformBuffers.size(); ++i) {
		bloomUniformBuffers[i] = graphicsCore->CreateUniformBuffer(uniformBufferCreateInfo);
	}
}

void DeferredRenderer::CreateSsrRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	if (imageSet.ssrRenderTarget != nullptr) {
		graphicsCore->DeleteRenderTarget(imageSet.ssrRenderTarget);
	}

	if (imageSet.ssrDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.ssrDescriptorSet);
	}
	
	GraphicsAPI::RenderTarget::CreateInfo ssrRenderTargetCreateInfo{ GraphicsAPI::ColorFormat::RGBA16, framebufferWidth, framebufferHeight, true, true, "SSR Render Target" };
	imageSet.ssrRenderTarget = graphicsCore->CreateRenderTarget(ssrRenderTargetCreateInfo);

	std::array<GraphicsAPI::DescriptorSet::Binding, 6> descriptorBindings;
	descriptorBindings[0].itemPtr = imageSet.globalUniformBufferObject;
	descriptorBindings[1].itemPtr = imageSet.ssrRenderTarget;
	descriptorBindings[2].itemPtr = imageSet.litHdrRenderTarget;
	descriptorBindings[3].itemPtr = imageSet.gbufferDepthTarget;
	descriptorBindings[4].itemPtr = imageSet.gbufferNormalRenderTarget;
	descriptorBindings[5].itemPtr = imageSet.gbufferSpecularRoughnessRenderTarget;

	GraphicsAPI::DescriptorSet::CreateInfo descriptorSetCreateInfo{};
	descriptorSetCreateInfo.debugName = "SSR Descriptor Set";
	descriptorSetCreateInfo.layout = ssrDescriptorSetLayout;
	descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
	descriptorSetCreateInfo.bindings = descriptorBindings.data();

	imageSet.ssrDescriptorSet = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
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

	imageSet.bloomRenderTargets.resize(bloomStoredMipLevelCount * 2);
	imageSet.bloomDescriptorSets.resize(bloomStoredMipLevelCount * 2 - 2);

	GraphicsAPI::RenderTarget::CreateInfo bloomRenderTargetCreateInfo{ GraphicsAPI::ColorFormat::RGBA32, framebufferWidth, framebufferHeight, true, true, "Bloom Render Target" };

	for (uint32_t i = 0; i < bloomStoredMipLevelCount; ++i) {
		std::string bloomRenderTargetName = std::string("Bloom Render Target Downscale Mip ") + std::to_string(i);
		bloomRenderTargetCreateInfo.debugName = bloomRenderTargetName.c_str();
		imageSet.bloomRenderTargets[i] = graphicsCore->CreateRenderTarget(bloomRenderTargetCreateInfo);
		bloomRenderTargetCreateInfo.width = bloomRenderTargetCreateInfo.width / 2;
		bloomRenderTargetCreateInfo.height = bloomRenderTargetCreateInfo.height / 2;
	}

	bloomRenderTargetCreateInfo.width = framebufferWidth;
	bloomRenderTargetCreateInfo.height = framebufferHeight;

	for (uint32_t i = 0; i < bloomStoredMipLevelCount; ++i) {
		std::string bloomRenderTargetName = std::string("Bloom Render Target Upscale Mip ") + std::to_string(i);
		bloomRenderTargetCreateInfo.debugName = bloomRenderTargetName.c_str();
		imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i] = graphicsCore->CreateRenderTarget(bloomRenderTargetCreateInfo);
		bloomRenderTargetCreateInfo.width = bloomRenderTargetCreateInfo.width / 2;
		bloomRenderTargetCreateInfo.height = bloomRenderTargetCreateInfo.height / 2;
	}

	std::array<GraphicsAPI::DescriptorSet::Binding, 4> descriptorBindings;
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
	if (bloomStoredMipLevelCount > 1) {
		std::string bloomDescriptorName = fmt::format("Bloom DS Filter [{}]", imageSetIndex);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[1];
		descriptorBindings[2].itemPtr = imageSet.litHdrRenderTarget;
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Downsample;
	for (size_t i = 1; i < bloomStoredMipLevelCount - 1; ++i) {
		std::string bloomDescriptorName = fmt::format("Bloom DS Downsample [{}]({})", imageSetIndex, i);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[i + 1];
		descriptorBindings[2].itemPtr = imageSet.bloomRenderTargets[i];
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		std::string bloomDescriptorName = fmt::format("Bloom DS First Upsample [{}])", imageSetIndex);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[bloomStoredMipLevelCount * 2 - 1];
		descriptorBindings[2].itemPtr = imageSet.bloomRenderTargets[bloomStoredMipLevelCount - 2];
		descriptorBindings[3].itemPtr = imageSet.bloomRenderTargets[bloomStoredMipLevelCount - 1];
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Upsample;
	for (size_t i = bloomStoredMipLevelCount - 2; i >= 1; --i) {
		std::string bloomDescriptorName = fmt::format("Bloom DS Upsample [{}]({})", imageSetIndex, i);
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UpdateBuffer(&bloomUboStruct);
		descriptorBindings[0].itemPtr = bloomUniformBuffers[bloomDescriptorSetIndex];
		descriptorBindings[1].itemPtr = imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i];
		descriptorBindings[3].itemPtr = imageSet.bloomRenderTargets[i];
		descriptorBindings[2].itemPtr = imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 1];
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void DeferredRenderer::UpdateBloomUBO() {
	std::vector<glm::vec2> mipSizes(bloomMipLevelCount);
	float mipWidth = static_cast<float>(renderWidth);
	float mipHeight = static_cast<float>(renderHeight);

	float widthMultiplier = static_cast<float>(renderWidth) / static_cast<float>(framebufferWidth);
	float heightMultiplier = static_cast<float>(renderHeight) / static_cast<float>(framebufferHeight);
	for (uint32_t i = 0; i < bloomMipLevelCount; ++i) {
		mipSizes[i] = glm::vec2(widthMultiplier / static_cast<float>(mipWidth), heightMultiplier / static_cast<float>(mipHeight));
		mipWidth = mipWidth / 2.0f;
		mipHeight = mipHeight / 2.0f;
	}

	// Threshold values sourced from: https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/
	const float threshold = 1.0f;
	const float softThreshold = 0.5f;
	const float knee = threshold * softThreshold;
	const float thresholdBias = 0.00001f;

	BloomUboStruct bloomUboStruct{};
	bloomUboStruct.thresholdFilter = { threshold, threshold - knee, 2.0f * knee, 0.25f / (knee + thresholdBias) };
	bloomUboStruct.levelOfDetail = 0.0f;
	bloomUboStruct.filterRadius = 0.005f;

	uint32_t bloomUboIndex = 0;

	bloomUboStruct.stage = BloomStage::Filter;
	if (bloomMipLevelCount > 1) {
		bloomUboStruct.outReciprocalImgSize = mipSizes[0];
		bloomUniformBuffers[bloomUboIndex++]->UpdateBuffer(&bloomUboStruct);

		bloomUboStruct.stage = BloomStage::Downsample;
		for (size_t i = 1; i < bloomMipLevelCount - 1; ++i) {
			bloomUboStruct.outReciprocalImgSize = mipSizes[i + 1];
			bloomUniformBuffers[bloomUboIndex++]->UpdateBuffer(&bloomUboStruct);
		}

		bloomUboStruct.stage = BloomStage::Upsample;
		{
			bloomUboStruct.outReciprocalImgSize = mipSizes[bloomMipLevelCount - 1];
			bloomUboStruct.inReciprocalImgSize = mipSizes[bloomMipLevelCount - 2];
			bloomUniformBuffers[bloomFirstUpsampleIndex]->UpdateBuffer(&bloomUboStruct);
		}

		bloomUboIndex = static_cast<uint32_t>((bloomStoredMipLevelCount * 2) - bloomMipLevelCount);
		for (size_t i = bloomMipLevelCount - 2; i >= 1; --i) {
			bloomUboStruct.outReciprocalImgSize = mipSizes[i];
			bloomUboStruct.inReciprocalImgSize = mipSizes[i - 1];
			bloomUniformBuffers[bloomUboIndex++]->UpdateBuffer(&bloomUboStruct);
		}

	}
}

void DeferredRenderer::UpdateBloomDescriptorSet(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	if (bloomMipLevelCount <= 2) {
		return;
	}

	std::array<DescriptorSet::Binding, 3> bindings;
	bindings[0].itemPtr = imageSet.bloomRenderTargets[(bloomStoredMipLevelCount * 2) - bloomMipLevelCount + 2];
	bindings[1].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount - 2];
	bindings[2].itemPtr = imageSet.bloomRenderTargets[bloomMipLevelCount - 1];

	imageSet.bloomDescriptorSets[bloomFirstUpsampleIndex]->ChangeBindings(bindings.data(), static_cast<uint32_t>(bindings.size()), 1);
}

void DeferredRenderer::CreateUniformBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		DeferredRendererImageSet& imageSet = deferredRendererImageSets[i];

		UniformBuffer::CreateInfo globalUniformBufferObjectCi{};
		globalUniformBufferObjectCi.debugName = "EngineUbo";
		globalUniformBufferObjectCi.isDynamic = true;
		globalUniformBufferObjectCi.size = sizeof(EngineUboStruct);
		imageSet.globalUniformBufferObject = graphicsCore->CreateUniformBuffer(globalUniformBufferObjectCi);

		UniformBuffer::CreateInfo debugUniformBufferObjectCi{};
		debugUniformBufferObjectCi.debugName = "DebugUbo";
		debugUniformBufferObjectCi.isDynamic = true;
		debugUniformBufferObjectCi.size = sizeof(DebugUboData);
		imageSet.debugUniformBufferObject = graphicsCore->CreateUniformBuffer(debugUniformBufferObjectCi);
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
	litHdrRenderTargetBinding.type = BindingType::RenderTexture;
	// TODO: Just using vertex for now to get resolution cuz I'm lazy. Remove this eventually and use a uniform buffer.
	litHdrRenderTargetBinding.stages = ShaderStageBit::Vertex | ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbufferDepthBinding{};
	gbufferDepthBinding.bindingId = 1;
	gbufferDepthBinding.count = 1;
	gbufferDepthBinding.type = BindingType::DepthTexture;
	gbufferDepthBinding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer0Binding{};
	gbuffer0Binding.bindingId = 2;
	gbuffer0Binding.count = 1;
	gbuffer0Binding.type = BindingType::RenderTexture;
	gbuffer0Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer1Binding{};
	gbuffer1Binding.bindingId = 3;
	gbuffer1Binding.count = 1;
	gbuffer1Binding.type = BindingType::RenderTexture;
	gbuffer1Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::Binding gbuffer2Binding{};
	gbuffer2Binding.bindingId = 4;
	gbuffer2Binding.count = 1;
	gbuffer2Binding.type = BindingType::RenderTexture;
	gbuffer2Binding.stages = ShaderStageBit::Fragment;

	DescriptorSetLayout::CreateInfo engineDescriptorSetLayoutCreateInfo{};
	engineDescriptorSetLayoutCreateInfo.debugName = "Engine UBO Set Layout";
	engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
	engineDescriptorSetLayoutCreateInfo.bindings = &engineUboBinding;
	engineDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 5> tonemapDescriptorSetLayoutBindings{};
	tonemapDescriptorSetLayoutBindings[0] = engineUboBinding;
	tonemapDescriptorSetLayoutBindings[1] = litHdrRenderTargetBinding;
	tonemapDescriptorSetLayoutBindings[2] = { 2, 1, BindingType::RenderTexture, ShaderStageBit::Fragment }; // Bloom Texture
	tonemapDescriptorSetLayoutBindings[3] = { 3, 1, BindingType::DepthTexture, ShaderStageBit::Fragment };	// Depth Texture
	tonemapDescriptorSetLayoutBindings[4] = { 4, 1, BindingType::UniformBuffer, ShaderStageBit::Fragment };	// Post Process Uniform Buffer

	DescriptorSetLayout::CreateInfo tonemapDescriptorSetLayoutCreateInfo{};
	tonemapDescriptorSetLayoutCreateInfo.debugName = "Tonemap Descriptor Set Layout";
	tonemapDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetLayoutBindings.size());
	tonemapDescriptorSetLayoutCreateInfo.bindings = tonemapDescriptorSetLayoutBindings.data();
	tonemapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(tonemapDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 7> debugDescriptorSetLayoutBindings{};
	debugDescriptorSetLayoutBindings[0] = engineUboBinding;
	debugDescriptorSetLayoutBindings[1] = gbufferDepthBinding;
	debugDescriptorSetLayoutBindings[2] = gbuffer0Binding;
	debugDescriptorSetLayoutBindings[3] = gbuffer1Binding;
	debugDescriptorSetLayoutBindings[4] = gbuffer2Binding;
	debugDescriptorSetLayoutBindings[5] = { 5, 1, BindingType::RenderTexture, ShaderStageBit::Fragment }; // Ambient Occlusion Texture
	debugDescriptorSetLayoutBindings[6] = { 6, 1, BindingType::UniformBuffer, ShaderStageBit::Fragment }; // Post Process Uniform Buffer

	DescriptorSetLayout::CreateInfo debugDescriptorSetLayoutCreateInfo{};
	debugDescriptorSetLayoutCreateInfo.debugName = "Debug Descriptor Set Layout";
	debugDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(debugDescriptorSetLayoutBindings.size());
	debugDescriptorSetLayoutCreateInfo.bindings = debugDescriptorSetLayoutBindings.data();
	debugDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(debugDescriptorSetLayoutCreateInfo);

	std::array<DescriptorSetLayout::Binding, 5> lightingDescriptorSetLayoutBindings{};
	lightingDescriptorSetLayoutBindings[0] = engineUboBinding;
	lightingDescriptorSetLayoutBindings[1] = gbufferDepthBinding;
	lightingDescriptorSetLayoutBindings[2] = gbuffer0Binding;
	lightingDescriptorSetLayoutBindings[3] = gbuffer1Binding;
	lightingDescriptorSetLayoutBindings[4] = gbuffer2Binding;

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
	shadowMappedLightBindings[0] = { 0, 1, BindingType::UniformBuffer, ShaderStageBit::Fragment };
	shadowMappedLightBindings[1] = { 1, 1, BindingType::DepthTexture, ShaderStageBit::Fragment };

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

	{
		std::array<DescriptorSetLayout::Binding, 2> ambientOcclusionInputLayoutBinding{};
		ambientOcclusionInputLayoutBinding[0].bindingId = 0;
		ambientOcclusionInputLayoutBinding[0].count = 1;
		ambientOcclusionInputLayoutBinding[0].type = BindingType::RenderTexture;
		ambientOcclusionInputLayoutBinding[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ambientOcclusionInputLayoutBinding[1].bindingId = 1;
		ambientOcclusionInputLayoutBinding[1].count = 1;
		ambientOcclusionInputLayoutBinding[1].type = BindingType::Texture;
		ambientOcclusionInputLayoutBinding[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		DescriptorSetLayout::CreateInfo ambientOcclusionInputLayoutCreateInfo{};
		ambientOcclusionInputLayoutCreateInfo.debugName = "Ambient Occlusion Descriptor Set Layout";
		ambientOcclusionInputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ambientOcclusionInputLayoutBinding.size());
		ambientOcclusionInputLayoutCreateInfo.bindings = ambientOcclusionInputLayoutBinding.data();
		ambientOcclusionDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ambientOcclusionInputLayoutCreateInfo);
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
}

void DeferredRenderer::CreateDescriptorSets(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSet::Binding engineUboBinding{ imageSet.globalUniformBufferObject };
	DescriptorSet::Binding litHdrRenderTargetBinding{ imageSet.litHdrRenderTarget };
	DescriptorSet::Binding gbufferDepthBinding{ imageSet.gbufferDepthTarget };
	DescriptorSet::Binding gbufferAlbedoBinding{ imageSet.gbufferAlbedoRenderTarget };
	DescriptorSet::Binding gbufferNormalBinding{ imageSet.gbufferNormalRenderTarget };
	DescriptorSet::Binding gbufferSpecRoughnessBinding{ imageSet.gbufferSpecularRoughnessRenderTarget };

	DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
	engineDescriptorSetCreateInfo.debugName = "Engine UBO Descriptor Set";
	engineDescriptorSetCreateInfo.layout = engineDescriptorSetLayout;
	engineDescriptorSetCreateInfo.bindingCount = 1;
	engineDescriptorSetCreateInfo.bindings = &engineUboBinding;
	imageSet.engineDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 5> tonemapDescriptorSetBindings{};
	tonemapDescriptorSetBindings[0] = engineUboBinding;
	tonemapDescriptorSetBindings[1] = litHdrRenderTargetBinding;
	tonemapDescriptorSetBindings[2] = { imageSet.bloomRenderTargets[bloomMipLevelCount + 1] };
	tonemapDescriptorSetBindings[3] = gbufferDepthBinding;
	tonemapDescriptorSetBindings[4] = { imageSet.tonemapPostProcessingUniformBufferObject };

	DescriptorSet::CreateInfo tonemapDescriptorSetCreateInfo{};
	tonemapDescriptorSetCreateInfo.debugName = "Tonemap Descriptor Set";
	tonemapDescriptorSetCreateInfo.layout = tonemapDescriptorSetLayout;
	tonemapDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetBindings.size());
	tonemapDescriptorSetCreateInfo.bindings = tonemapDescriptorSetBindings.data();
	imageSet.tonemapDescriptorSet = graphicsCore->CreateDescriptorSet(tonemapDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 7> debugDescriptorSetBindings{};
	debugDescriptorSetBindings[0] = engineUboBinding;
	debugDescriptorSetBindings[1] = gbufferDepthBinding;
	debugDescriptorSetBindings[2] = gbufferAlbedoBinding;
	debugDescriptorSetBindings[3] = gbufferNormalBinding;
	debugDescriptorSetBindings[4] = gbufferSpecRoughnessBinding;
	debugDescriptorSetBindings[5] = { imageSet.ambientOcclusionRenderTarget };
	debugDescriptorSetBindings[6] = { imageSet.debugUniformBufferObject };

	DescriptorSet::CreateInfo debugDescriptorSetCreateInfo{};
	debugDescriptorSetCreateInfo.debugName = "Debug Descriptor Set";
	debugDescriptorSetCreateInfo.layout = debugDescriptorSetLayout;
	debugDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(debugDescriptorSetBindings.size());
	debugDescriptorSetCreateInfo.bindings = debugDescriptorSetBindings.data();
	imageSet.debugDescriptorSet = graphicsCore->CreateDescriptorSet(debugDescriptorSetCreateInfo);

	std::array<DescriptorSet::Binding, 5> lightingDescriptorSetBindings{};
	lightingDescriptorSetBindings[0] = engineUboBinding;
	lightingDescriptorSetBindings[1] = gbufferDepthBinding;
	lightingDescriptorSetBindings[2] = gbufferAlbedoBinding;
	lightingDescriptorSetBindings[3] = gbufferNormalBinding;
	lightingDescriptorSetBindings[4] = gbufferSpecRoughnessBinding;

	DescriptorSet::CreateInfo lightingDescriptorSetCreateInfo{};
	lightingDescriptorSetCreateInfo.debugName = "Point Light Descriptor Set";
	lightingDescriptorSetCreateInfo.layout = lightingDescriptorSetLayout;
	lightingDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(lightingDescriptorSetBindings.size());
	lightingDescriptorSetCreateInfo.bindings = lightingDescriptorSetBindings.data();
	imageSet.lightingDescriptorSet = graphicsCore->CreateDescriptorSet(lightingDescriptorSetCreateInfo);

	{
		std::array<DescriptorSet::Binding, 2> aoInputBinding = { imageSet.ambientOcclusionRenderTarget, brdfLut };

		DescriptorSet::CreateInfo aoInputCreateInfo{};
		aoInputCreateInfo.debugName = "Ambient Occlusion Descriptor Set";
		aoInputCreateInfo.layout = ambientOcclusionDescriptorSetLayout;
		aoInputCreateInfo.bindingCount = static_cast<uint32_t>(aoInputBinding.size());
		aoInputCreateInfo.bindings = aoInputBinding.data();
		imageSet.ambientOcclusionDescriptorSet = graphicsCore->CreateDescriptorSet(aoInputCreateInfo);
	}

	{
		DescriptorSet::Binding environmentMapBinding{ nullptr };

		DescriptorSet::CreateInfo environmentMapDescriptorCreateInfo{};
		environmentMapDescriptorCreateInfo.debugName = "Environment Map Input Descriptor Set";
		environmentMapDescriptorCreateInfo.layout = environmentMapDescriptorSetLayout;
		environmentMapDescriptorCreateInfo.bindingCount = 0;
		environmentMapDescriptorCreateInfo.bindings = nullptr;
		environmentMapDescriptorSet = graphicsCore->CreateDescriptorSet(environmentMapDescriptorCreateInfo);
	}
}

void DeferredRenderer::UpdateDescriptorSets(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	DescriptorSet::Binding engineUboBinding{ imageSet.globalUniformBufferObject };
	DescriptorSet::Binding litHdrRenderTargetBinding{ imageSet.litHdrRenderTarget };
	DescriptorSet::Binding gbufferDepthBinding{ imageSet.gbufferDepthTarget };
	DescriptorSet::Binding gbufferAlbedoBinding{ imageSet.gbufferAlbedoRenderTarget };
	DescriptorSet::Binding gbufferNormalBinding{ imageSet.gbufferNormalRenderTarget };
	DescriptorSet::Binding gbufferSpecRoughnessBinding{ imageSet.gbufferSpecularRoughnessRenderTarget };
	imageSet.engineDescriptorSet->ChangeBindings(&engineUboBinding, 1);

	{
		std::array<DescriptorSet::Binding, 5> tonemapDescriptorSetBindings{};
		tonemapDescriptorSetBindings[0] = engineUboBinding;
		tonemapDescriptorSetBindings[1] = litHdrRenderTargetBinding;
		tonemapDescriptorSetBindings[2] = { imageSet.bloomRenderTargets[bloomMipLevelCount + 1] };
		tonemapDescriptorSetBindings[3] = gbufferDepthBinding;
		tonemapDescriptorSetBindings[4] = { imageSet.tonemapPostProcessingUniformBufferObject };

		imageSet.tonemapDescriptorSet->ChangeBindings(tonemapDescriptorSetBindings.data(), static_cast<uint32_t>(tonemapDescriptorSetBindings.size()));
	}

	{
		std::array<DescriptorSet::Binding, 5> lightingDescriptorSetBindings{};
		lightingDescriptorSetBindings[0] = engineUboBinding;
		lightingDescriptorSetBindings[1] = gbufferDepthBinding;
		lightingDescriptorSetBindings[2] = gbufferAlbedoBinding;
		lightingDescriptorSetBindings[3] = gbufferNormalBinding;
		lightingDescriptorSetBindings[4] = gbufferSpecRoughnessBinding;

		imageSet.lightingDescriptorSet->ChangeBindings(lightingDescriptorSetBindings.data(), static_cast<uint32_t>(lightingDescriptorSetBindings.size()));
	}

	{
		DescriptorSet::Binding ssaoInputBinding{ imageSet.ambientOcclusionRenderTarget };
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

	const int gbufferColorCount = 3;
	std::array<RenderPass::AttachmentInfo, gbufferColorCount> gbufferColorAttachments{};
	gbufferColorAttachments[0] = { ColorFormat::RGBA8, true }; // Albedo
	gbufferColorAttachments[1] = { ColorFormat::RGBA16, true }; // Normal
	gbufferColorAttachments[2] = { ColorFormat::RGBA8, true }; // Specular RGB + Roughness Alpha

	if (gbufferRenderPass == nullptr) {
		RenderPass::CreateInfo gbufferRenderPassCreateInfo{};
		gbufferRenderPassCreateInfo.debugName = "GBuffer Render Pass";
		gbufferRenderPassCreateInfo.colorAttachments = gbufferColorAttachments.data();
		gbufferRenderPassCreateInfo.colorAttachmentCount = static_cast<uint32_t>(gbufferColorAttachments.size());
		gbufferRenderPassCreateInfo.depthFormat = depthFormat;
		gbufferRenderPassCreateInfo.shouldClearDepthOnLoad = true;
		gbufferRenderPass = graphicsCore->CreateRenderPass(gbufferRenderPassCreateInfo);
	}

	std::array<RenderTarget*, gbufferColorCount> gbufferRenderTargets{};
	DepthTarget::CreateInfo gbufferDepthImageCreateInfo(depthFormat, framebufferWidth, framebufferHeight, false, false, true, "GBuffer Depth Image");

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		{
			RenderTarget::CreateInfo gbufferRtCreateInfo{ gbufferColorAttachments[0].colorFormat, framebufferWidth, framebufferHeight, true, false, "GBuffer Albedo Image" };

			gbufferRenderTargets[0] = imageSet.gbufferAlbedoRenderTarget = graphicsCore->CreateRenderTarget(gbufferRtCreateInfo);

			gbufferRtCreateInfo.format = gbufferColorAttachments[1].colorFormat;
			gbufferRtCreateInfo.debugName = "GBuffer Normal Image";

			gbufferRenderTargets[1] = imageSet.gbufferNormalRenderTarget = graphicsCore->CreateRenderTarget(gbufferRtCreateInfo);

			gbufferRtCreateInfo.format = gbufferColorAttachments[2].colorFormat;
			gbufferRtCreateInfo.debugName = "GBuffer Specular + Roughness Image";

			gbufferRenderTargets[2] = imageSet.gbufferSpecularRoughnessRenderTarget = graphicsCore->CreateRenderTarget(gbufferRtCreateInfo);
		}

		imageSet.gbufferDepthTarget = graphicsCore->CreateDepthTarget(gbufferDepthImageCreateInfo);

		Framebuffer::CreateInfo gbufferCreateInfo{};
		gbufferCreateInfo.debugName = "G-Buffer Framebuffer";
		gbufferCreateInfo.width = framebufferWidth;
		gbufferCreateInfo.height = framebufferHeight;
		gbufferCreateInfo.renderPass = gbufferRenderPass;
		gbufferCreateInfo.renderTargetLists = gbufferRenderTargets.data();
		gbufferCreateInfo.numRenderTargetLists = static_cast<uint32_t>(gbufferRenderTargets.size());
		gbufferCreateInfo.depthTarget = imageSet.gbufferDepthTarget;

		imageSet.gbuffer = graphicsCore->CreateFramebuffer(gbufferCreateInfo);

		{
			GraphicsAPI::RenderTarget::CreateInfo ssaoRenderTargetCreateInfo{ ambientOcclusionFormat, framebufferWidth / 2, framebufferHeight / 2, true, false, "SSAO Render Target" };
			imageSet.ambientOcclusionRenderTarget = graphicsCore->CreateRenderTarget(ssaoRenderTargetCreateInfo);

			GraphicsAPI::Framebuffer::CreateInfo ssaoFramebufferCreateInfo{};
			ssaoFramebufferCreateInfo.debugName = "SSAO Framebuffer";
			ssaoFramebufferCreateInfo.width = framebufferWidth / 2;
			ssaoFramebufferCreateInfo.height = framebufferHeight / 2;
			ssaoFramebufferCreateInfo.renderTargetLists = &imageSet.ambientOcclusionRenderTarget;
			ssaoFramebufferCreateInfo.numRenderTargetLists = 1;
			ssaoFramebufferCreateInfo.depthTarget = nullptr;
			ssaoFramebufferCreateInfo.renderPass = ssaoRenderPass;
			imageSet.ambientOcclusionFramebuffer = graphicsCore->CreateFramebuffer(ssaoFramebufferCreateInfo);
		}
	}
}

void DeferredRenderer::CreateLitHDRFramebuffer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	RenderTarget::CreateInfo litHdrImagesCreateInfo = { Grindstone::GraphicsAPI::ColorFormat::RGBA16, framebufferWidth, framebufferHeight, true, false, "Lit HDR Color Image" };
	// DepthTarget::CreateInfo litHdrDepthImageCreateInfo(DepthFormat::D24_STENCIL_8, width, height, false, false, false, "Lit HDR Depth Image");

	RenderPass::AttachmentInfo attachment{ litHdrImagesCreateInfo.format , true };

	if (mainRenderPass == nullptr) {
		static float debugColor[4] = {0.3f, 0.6f, 0.9f, 1.0f};

		RenderPass::CreateInfo mainRenderPassCreateInfo{};
		mainRenderPassCreateInfo.debugName = "Main HDR Render Pass";
		mainRenderPassCreateInfo.colorAttachments = &attachment;
		mainRenderPassCreateInfo.colorAttachmentCount = 1;
		mainRenderPassCreateInfo.depthFormat = depthFormat;
		mainRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		memcpy(mainRenderPassCreateInfo.debugColor, debugColor, sizeof(float) * 4);
		mainRenderPass = graphicsCore->CreateRenderPass(mainRenderPassCreateInfo);
	}

	if (lightingRenderPass == nullptr) {
		static float debugColor[4] = { 1.0f, 0.9f, 0.5f, 1.0f };

		RenderPass::CreateInfo lightingRenderPassCreateInfo{};
		lightingRenderPassCreateInfo.debugName = "Deferred Light Render Pass";
		lightingRenderPassCreateInfo.colorAttachments = &attachment;
		lightingRenderPassCreateInfo.colorAttachmentCount = 1;
		lightingRenderPassCreateInfo.depthFormat = DepthFormat::None;
		lightingRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		memcpy(lightingRenderPassCreateInfo.debugColor, debugColor, sizeof(float) * 4);
		lightingRenderPass = graphicsCore->CreateRenderPass(lightingRenderPassCreateInfo);
	}

	if (forwardLitRenderPass == nullptr) {
		static float debugColor[4] = { 1.0f, 0.5f, 0.9f, 1.0f };

		RenderPass::CreateInfo forwardLitRenderPassCreateInfo{};
		forwardLitRenderPassCreateInfo.debugName = "Forward Lit Renderables Render Pass";
		forwardLitRenderPassCreateInfo.colorAttachments = &attachment;
		forwardLitRenderPassCreateInfo.colorAttachmentCount = 1;
		forwardLitRenderPassCreateInfo.depthFormat = depthFormat;
		forwardLitRenderPassCreateInfo.shouldClearDepthOnLoad = false;
		memcpy(forwardLitRenderPassCreateInfo.debugColor, debugColor, sizeof(float) * 4);
		forwardLitRenderPass = graphicsCore->CreateRenderPass(forwardLitRenderPassCreateInfo);
	}

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		imageSet.litHdrRenderTarget = graphicsCore->CreateRenderTarget(litHdrImagesCreateInfo);

		std::string framebufferName = std::string("Main HDR Framebuffer ") + std::to_string(i);
		Framebuffer::CreateInfo litHdrFramebufferCreateInfo{};
		litHdrFramebufferCreateInfo.debugName = framebufferName.c_str();
		litHdrFramebufferCreateInfo.width = framebufferWidth;
		litHdrFramebufferCreateInfo.height = framebufferHeight;
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
	pipelineCreateInfo.polygonFillMode = GraphicsAPI::PolygonFillMode::Fill;
	pipelineCreateInfo.cullMode = CullMode::Back;
	pipelineCreateInfo.scissorX = 0;
	pipelineCreateInfo.scissorY = 0;
	pipelineCreateInfo.hasDynamicScissor = true;
	pipelineCreateInfo.hasDynamicViewport = true;
	pipelineCreateInfo.vertexBindings = &vertexLightPositionLayout;
	pipelineCreateInfo.vertexBindingsCount = 1;

	std::vector<ShaderStageCreateInfo> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	Grindstone::Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;
	uint8_t shaderBits = static_cast<uint8_t>(ShaderStageBit::Vertex | ShaderStageBit::Fragment);
	 
	{
		if (!assetManager->LoadShaderSet(Uuid("3b3bc2c8-ac88-4fba-b9f9-704f86c1278c"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load ssao shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 2> ssaoLayouts{};
		ssaoLayouts[0] = lightingDescriptorSetLayout;
		ssaoLayouts[1] = ssaoInputDescriptorSetLayout;

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
		ssaoPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	pipelineCreateInfo.width = static_cast<float>(renderWidth);
	pipelineCreateInfo.height = static_cast<float>(renderHeight);
	pipelineCreateInfo.scissorW = framebufferWidth;
	pipelineCreateInfo.scissorH = framebufferHeight;

	{
		ShaderStageCreateInfo bloomShaderStageCreateInfo;
		std::vector<char> bloomFileData;

		if (!assetManager->LoadShaderStage(Uuid("8a2475b4-8731-456c-beb7-2d51db7914f9"), ShaderStage::Compute, bloomShaderStageCreateInfo, bloomFileData)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load bloom compute shader.");
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
		ShaderStageCreateInfo ssrShaderStageCreateInfo;
		std::vector<char> ssrFileData;

		if (!assetManager->LoadShaderStage(Uuid("cff2c843-6b35-4030-9a4b-464feb1e3365"), ShaderStage::Compute, ssrShaderStageCreateInfo, ssrFileData)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load SSR compute shader.");
			return;
		}

		ComputePipeline::CreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.debugName = "Screen Space Reflections Compute Pipeline";
		computePipelineCreateInfo.shaderFileName = ssrShaderStageCreateInfo.fileName;
		computePipelineCreateInfo.shaderContent = ssrFileData.data();
		computePipelineCreateInfo.shaderSize = static_cast<uint32_t>(ssrFileData.size());
		computePipelineCreateInfo.descriptorSetLayouts = &ssrDescriptorSetLayout;
		computePipelineCreateInfo.descriptorSetLayoutCount = 1;
		ssrPipeline = graphicsCore->CreateComputePipeline(computePipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(Uuid("5227a9a2-4a62-4f1b-9906-2b6acbf1b8d3"), shaderBits, 2, shaderStageCreateInfos, fileData)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load image based lighting shaders.");
			return;
		}

		std::array<GraphicsAPI::DescriptorSetLayout*, 3> iblLayouts{};
		iblLayouts[0] = lightingDescriptorSetLayout;
		iblLayouts[1] = ambientOcclusionDescriptorSetLayout;
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
			GPRINT_ERROR(LogSource::Rendering, "Could not load point light shaders.");
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
			GPRINT_ERROR(LogSource::Rendering, "Could not load spot light shaders.");
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
			GPRINT_ERROR(LogSource::Rendering, "Could not load directional light shaders.");
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
			Uuid("1f7b7b1e-2056-40d1-bb04-cbdc559325e8"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load debug shaders.");
			return;
		}

		pipelineCreateInfo.debugName = "Debug Pipeline";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = &debugDescriptorSetLayout;
		pipelineCreateInfo.descriptorSetLayoutCount = 1;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = targetRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = true;
		pipelineCreateInfo.isDepthTestEnabled = true;
		pipelineCreateInfo.isStencilEnabled = true;
		debugPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
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
			GPRINT_ERROR(LogSource::Rendering, "Could not load tonemapping shaders.");
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
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		tonemapPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(
			Uuid("fafd3e26-3b40-4c5d-88b5-4a906810bf19"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load depth of field separation shader.");
			return;
		}

		std::array<DescriptorSetLayout*, 2> layouts = {
			engineDescriptorSetLayout,
			dofSourceDescriptorSetLayout
		};

		pipelineCreateInfo.debugName = "Depth of Field (Separation Stage)";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = layouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineCreateInfo.colorAttachmentCount = 2;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = dofSeparationRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		dofSeparationPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(
			Uuid("65b37559-4286-4351-bf3a-d61e6a688d52"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load depth of field blur shader.");
			return;
		}

		std::array<DescriptorSetLayout*, 2> layouts = {
			engineDescriptorSetLayout,
			dofBlurDescriptorSetLayout
		};

		pipelineCreateInfo.debugName = "Depth of Field (Blur Stage)";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = layouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = dofBlurAndCombinationRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		dofBlurPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}

	shaderStageCreateInfos.clear();
	fileData.clear();

	{
		if (!assetManager->LoadShaderSet(
			Uuid("6c7f3e48-4439-47c5-a3a2-43441cd2657e"),
			shaderBits,
			2,
			shaderStageCreateInfos,
			fileData
		)) {
			GPRINT_ERROR(LogSource::Rendering, "Could not load depth of field combination shader.");
			return;
		}

		std::array<DescriptorSetLayout*, 3> layouts = {
			engineDescriptorSetLayout,
			dofSourceDescriptorSetLayout,
			dofCombinationDescriptorSetLayout
		};

		pipelineCreateInfo.debugName = "Depth of Field (Combination Stage)";
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = layouts.data();
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.blendMode = BlendMode::None;
		pipelineCreateInfo.renderPass = dofBlurAndCombinationRenderPass;
		pipelineCreateInfo.isDepthWriteEnabled = false;
		pipelineCreateInfo.isDepthTestEnabled = false;
		pipelineCreateInfo.isStencilEnabled = false;
		dofCombinationPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
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
			GPRINT_ERROR(LogSource::Rendering, "Could not load shadow mapping shaders.");
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
		pipelineCreateInfo.isDepthBiasEnabled = true;
		shadowMappingPipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);
	}
	pipelineCreateInfo.isDepthBiasEnabled = false;
}

void DeferredRenderer::RenderDepthOfField(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	ClearColorValue singleClearColor{ 0.0f, 0.0f, 0.0f, 0.f };

	ClearDepthStencil depthStencilClear;
	depthStencilClear.hasDepthStencilAttachment = false;

	currentCommandBuffer->BeginDebugLabelSection("Depth of Field Pass", nullptr);

	{
		std::array<ClearColorValue, 2> clearColors = {
			ClearColorValue{0.0f, 0.0f, 0.0f, 0.f},
			ClearColorValue{0.0f, 0.0f, 0.0f, 0.f}
		};

		currentCommandBuffer->BindRenderPass(
			dofSeparationRenderPass,
			imageSet.dofSeparationFramebuffer,
			renderWidth / 2,
			renderHeight / 2,
			clearColors.data(),
			static_cast<uint32_t>(clearColors.size()),
			depthStencilClear
		);

		std::array<DescriptorSet*, 2> descriptorSets = { imageSet.engineDescriptorSet, imageSet.dofSourceDescriptorSet };
		currentCommandBuffer->BindGraphicsPipeline(dofSeparationPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofSeparationPipeline,
			descriptorSets.data(),
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->UnbindRenderPass();
	}

	{
		currentCommandBuffer->BindRenderPass(
			dofBlurAndCombinationRenderPass,
			imageSet.dofNearBlurFramebuffer,
			renderWidth / 4,
			renderHeight / 4,
			&singleClearColor,
			1u,
			depthStencilClear
		);


		std::array<DescriptorSet*, 2> descriptorSets = { imageSet.engineDescriptorSet, imageSet.dofNearBlurDescriptorSet };

		currentCommandBuffer->BindGraphicsPipeline(dofBlurPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofBlurPipeline,
			descriptorSets.data(),
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->UnbindRenderPass();
	}

	{
		currentCommandBuffer->BindRenderPass(
			dofBlurAndCombinationRenderPass,
			imageSet.dofFarBlurFramebuffer,
			renderWidth / 4,
			renderHeight / 4,
			&singleClearColor,
			1u,
			depthStencilClear
		);

		std::array<DescriptorSet*, 2> descriptorSets = { imageSet.engineDescriptorSet, imageSet.dofFarBlurDescriptorSet };

		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofBlurPipeline,
			descriptorSets.data(),
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->UnbindRenderPass();
	}

	{
		currentCommandBuffer->BindRenderPass(
			dofBlurAndCombinationRenderPass,
			imageSet.dofCombinationFramebuffer,
			renderWidth,
			renderHeight,
			nullptr,
			0u,
			depthStencilClear
		);

		std::array<DescriptorSet*, 3> descriptorSets = { imageSet.engineDescriptorSet, imageSet.dofSourceDescriptorSet, imageSet.dofCombineDescriptorSet };
		currentCommandBuffer->BindGraphicsPipeline(dofCombinationPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofCombinationPipeline,
			descriptorSets.data(),
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->UnbindRenderPass();
	}

	currentCommandBuffer->EndDebugLabelSection();
}

void DeferredRenderer::RenderSsr(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	if (ssrPipeline == nullptr) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	currentCommandBuffer->BeginDebugLabelSection("Screen Space Reflections Pass", nullptr);
	currentCommandBuffer->BindComputePipeline(ssrPipeline);

	{
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.ssrRenderTarget, true);
		currentCommandBuffer->BindComputeDescriptorSet(ssrPipeline, &imageSet.ssrDescriptorSet, 1);
		currentCommandBuffer->DispatchCompute(renderWidth, renderHeight, 1);
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.ssrRenderTarget, false);
	}
	currentCommandBuffer->EndDebugLabelSection();
}

void DeferredRenderer::RenderBloom(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	if (bloomPipeline == nullptr || bloomMipLevelCount <= 2) {
		return;
	}

	static float debugColor[4] = { 1.0f, 0.6f, 0.55f, 1.0f };
	static float debugColorLevel2[4] = { 0.9f, 0.5f, 0.4f, 1.0f };

	currentCommandBuffer->BeginDebugLabelSection("Bloom Pass", debugColor);
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	currentCommandBuffer->BindComputePipeline(bloomPipeline);
	uint32_t groupCountX = static_cast<uint32_t>(std::ceil(renderWidth / 4.0f));
	uint32_t groupCountY = static_cast<uint32_t>(std::ceil(renderHeight / 4.0f));
	uint32_t descriptorSetIndex = 0;

	currentCommandBuffer->BeginDebugLabelSection("Bloom First Downsample", debugColor);
	{
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[1], true);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}
	currentCommandBuffer->EndDebugLabelSection();

	uint32_t mipWidth = static_cast<uint32_t>(renderWidth);
	uint32_t mipHeight = static_cast<uint32_t>(renderHeight);

	std::vector<uint32_t> mipWidths(bloomMipLevelCount);
	std::vector<uint32_t> mipHeights(bloomMipLevelCount);
	mipWidths[0] = mipWidth;
	mipHeights[0] = mipHeight;

	currentCommandBuffer->BeginDebugLabelSection("Bloom Downsamples", debugColor);
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
	currentCommandBuffer->EndDebugLabelSection();

	currentCommandBuffer->BeginDebugLabelSection("Bloom First Upsample", debugColor);
	{
		groupCountX = static_cast<uint32_t>(std::ceil(mipWidths[bloomMipLevelCount - 2] / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeights[bloomMipLevelCount - 2] / 4.0f));

		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomMipLevelCount - 1], false);
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[(bloomStoredMipLevelCount * 2) - bloomMipLevelCount + 2], true);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[bloomFirstUpsampleIndex], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}
	currentCommandBuffer->EndDebugLabelSection();

	currentCommandBuffer->BeginDebugLabelSection("Bloom Upsamples", debugColor);
	descriptorSetIndex = static_cast<uint32_t>((bloomStoredMipLevelCount * 2) - bloomMipLevelCount);
	for (size_t i = bloomMipLevelCount - 3; i != SIZE_MAX; --i) {
		mipWidth = mipWidths[i];
		mipHeight = mipHeights[i];
		groupCountX = static_cast<uint32_t>(std::ceil(mipWidth / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeight / 4.0f));

		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 1], true);
		currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 2], false);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}
	currentCommandBuffer->EndDebugLabelSection();

	currentCommandBuffer->EndDebugLabelSection();

	currentCommandBuffer->WaitForComputeMemoryBarrier(imageSet.bloomRenderTargets[bloomStoredMipLevelCount + 1], false);
}

void DeferredRenderer::RenderLights(
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
		currentCommandBuffer->BeginDebugLabelSection("Image Based Lighting", nullptr);
		currentCommandBuffer->BindGraphicsPipeline(imageBasedLightingPipeline);

		auto view = registry.view<const EnvironmentMapComponent>();

		bool hasEnvMap = false;
		view.each([&](const EnvironmentMapComponent& environmentMapComponent) {
			if (currentEnvironmentMapUuid == environmentMapComponent.specularTexture.uuid) {
				hasEnvMap = true;
				return;
			}

			currentEnvironmentMapUuid = environmentMapComponent.specularTexture.uuid;
			Grindstone::TextureAsset* texAsset = environmentMapComponent.specularTexture.Get();
			if (texAsset != nullptr) {
				Texture* tex = texAsset->texture;
				hasEnvMap = true;

				GraphicsAPI::DescriptorSet::Binding binding{ tex };
				environmentMapDescriptorSet->ChangeBindings(&binding, 1);
			}
		});

		if (hasEnvMap) {
			std::array<GraphicsAPI::DescriptorSet*, 3> iblDescriptors{};
			iblDescriptors[0] = imageSet.lightingDescriptorSet;
			iblDescriptors[1] = imageSet.ambientOcclusionDescriptorSet;
			iblDescriptors[2] = environmentMapDescriptorSet;
			currentCommandBuffer->BindGraphicsDescriptorSet(imageBasedLightingPipeline, iblDescriptors.data(), static_cast<uint32_t>(iblDescriptors.size()));
			currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		}
		currentCommandBuffer->EndDebugLabelSection();
	}

	if (pointLightPipeline != nullptr) {
		currentCommandBuffer->BeginDebugLabelSection("Point Lighting", nullptr);
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
			currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		});
		currentCommandBuffer->EndDebugLabelSection();
	}

	if (spotLightPipeline != nullptr) {
		currentCommandBuffer->BeginDebugLabelSection("Spot Lighting", nullptr);
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
			currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		});
		currentCommandBuffer->EndDebugLabelSection();
	}

	if (directionalLightPipeline != nullptr) {
		currentCommandBuffer->BeginDebugLabelSection("Directional Lighting", nullptr);
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
			currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
			currentCommandBuffer->EndDebugLabelSection();
		});
	}
}

void DeferredRenderer::RenderSsao(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* commandBuffer) {
	if (ssaoPipeline == nullptr) {
		return;
	}

	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	ClearColorValue clearColorAttachment = { 16.0f, 16.0f, 16.0f, 16.0f };
	ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = false;

	commandBuffer->BindRenderPass(
		ssaoRenderPass,
		imageSet.ambientOcclusionFramebuffer,
		imageSet.ambientOcclusionFramebuffer->GetWidth(),
		imageSet.ambientOcclusionFramebuffer->GetHeight(),
		&clearColorAttachment,
		1,
		clearDepthStencil
	);

	uint32_t halfWidth = renderWidth / 2u;
	uint32_t halfHeight = renderHeight / 2u;

	commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(halfWidth), static_cast<float>(halfHeight), 0.0f, 1.0f);
	commandBuffer->SetScissor(0, 0, halfWidth, halfHeight);

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	std::array<GraphicsAPI::DescriptorSet*, 2> ssaoDescriptors{};
	ssaoDescriptors[0] = imageSet.lightingDescriptorSet;
	ssaoDescriptors[1] = ssaoInputDescriptorSet;

	commandBuffer->BindGraphicsPipeline(ssaoPipeline);
	commandBuffer->BindGraphicsDescriptorSet(ssaoPipeline, ssaoDescriptors.data(), static_cast<uint32_t>(ssaoDescriptors.size()));
	commandBuffer->DrawIndices(0, 6, 0, 1, 0);
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

void DeferredRenderer::PostProcess(
	uint32_t imageIndex,
	GraphicsAPI::Framebuffer* framebuffer,
	GraphicsAPI::CommandBuffer* currentCommandBuffer
) {
	GRIND_PROFILE_FUNC();

	DeferredRendererImageSet& imageSet = deferredRendererImageSets[imageIndex];

	// RenderDepthOfField(imageSet, currentCommandBuffer);
	RenderBloom(imageSet, currentCommandBuffer);

	ClearColorValue clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = true;
	auto renderPass = framebuffer->GetRenderPass();

	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, framebuffer->GetWidth(), framebuffer->GetHeight(), &clearColor, 1, clearDepthStencil);

	if (tonemapPipeline != nullptr) {
		imageSet.tonemapPostProcessingUniformBufferObject->UpdateBuffer(&postProcessUboData);

		currentCommandBuffer->BindGraphicsDescriptorSet(tonemapPipeline, &imageSet.tonemapDescriptorSet, 1);
		currentCommandBuffer->BindGraphicsPipeline(tonemapPipeline);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
	}

	currentCommandBuffer->UnbindRenderPass();

	currentCommandBuffer->BlitDepthImage(imageSet.gbufferDepthTarget, framebuffer->GetDepthTarget());
}

void DeferredRenderer::Debug(
	uint32_t imageIndex,
	GraphicsAPI::Framebuffer* framebuffer,
	GraphicsAPI::CommandBuffer* currentCommandBuffer
) {
	GRIND_PROFILE_FUNC();

	DeferredRendererImageSet& imageSet = deferredRendererImageSets[imageIndex];

	ClearColorValue clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;
	clearDepthStencil.hasDepthStencilAttachment = false;

	RenderPass* renderPass = framebuffer->GetRenderPass();
	currentCommandBuffer->BindRenderPass(renderPass, framebuffer, framebuffer->GetWidth(), framebuffer->GetHeight(), &clearColor, 1, clearDepthStencil);

	if (debugPipeline != nullptr) {
		imageSet.debugUniformBufferObject->UpdateBuffer(&debugUboData);

		currentCommandBuffer->BindGraphicsDescriptorSet(debugPipeline, &imageSet.debugDescriptorSet, 1);
		currentCommandBuffer->BindGraphicsPipeline(debugPipeline);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
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
	engineUboStruct.framebufferResolution = glm::vec2(framebufferWidth, framebufferHeight);
	engineUboStruct.renderResolution = glm::vec2(renderWidth, renderHeight);
	engineUboStruct.renderScale = glm::vec2(static_cast<float>(renderWidth) / framebufferWidth, static_cast<float>(renderHeight) / framebufferHeight);
	engineUboStruct.time = static_cast<float>(engineCore.GetTimeSinceLaunch());
	imageSet.globalUniformBufferObject->UpdateBuffer(&engineUboStruct);

	if (renderMode == DeferredRenderMode::Default) {
		RenderShadowMaps(commandBuffer, registry);
	}

	assetManager->SetEngineDescriptorSet(imageSet.engineDescriptorSet);

	{
		std::array<ClearColorValue, 3> clearColors = {
			ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
			ClearColorValue{0.0f, 0.0f, 0.0f, 1.f},
			ClearColorValue{0.0f, 0.0f, 0.0f, 1.f}
		};

		ClearDepthStencil clearDepthStencil;
		clearDepthStencil.depth = 1.0f;
		clearDepthStencil.stencil = 0;
		clearDepthStencil.hasDepthStencilAttachment = true;

		commandBuffer->BindRenderPass(
			gbufferRenderPass,
			imageSet.gbuffer,
			renderWidth, renderHeight,
			clearColors.data(),
			static_cast<uint32_t>(clearColors.size()),
			clearDepthStencil
		);
	}

	commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderWidth), static_cast<float>(renderHeight));
	commandBuffer->SetScissor(0, 0, renderWidth, renderHeight);

	assetManager->RenderQueue(commandBuffer, "Opaque");
	commandBuffer->UnbindRenderPass();

	if (renderMode == DeferredRenderMode::Default || renderMode == DeferredRenderMode::AmbientOcclusion) {
		RenderSsao(imageSet, commandBuffer);
	}

	commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderWidth), static_cast<float>(renderHeight));
	commandBuffer->SetScissor(0, 0, renderWidth, renderHeight);

	{
		ClearColorValue clearColor{ 0.0f, 0.0f, 0.0f, 1.f };
		ClearDepthStencil clearDepthStencil;

		commandBuffer->BindRenderPass(mainRenderPass, imageSet.litHdrFramebuffer, renderWidth, renderHeight, &clearColor, 1, clearDepthStencil);

		if (renderMode == DeferredRenderMode::Default) {
			commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
			commandBuffer->BindIndexBuffer(indexBuffer);
			RenderLights(imageIndex, commandBuffer, registry);
		}

		assetManager->RenderQueue(commandBuffer, "Unlit");

		if (renderMode == DeferredRenderMode::Default) {
			assetManager->RenderQueue(commandBuffer, "Skybox");
			assetManager->RenderQueue(commandBuffer, "Transparent");
		}

		commandBuffer->UnbindRenderPass();
	}

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	// RenderSsr(imageSet, commandBuffer);

	if (renderMode == DeferredRenderMode::Default) {
		PostProcess(imageIndex, outputFramebuffer, commandBuffer);
	}
	else {
		debugUboData.renderMode = static_cast<uint16_t>(renderMode);
		Debug(imageIndex, outputFramebuffer, commandBuffer);
	}
}

uint16_t DeferredRenderer::GetRenderModeCount() const {
	return static_cast<uint16_t>(renderModes.size());
}

const Grindstone::BaseRenderer::RenderMode* DeferredRenderer::GetRenderModes() const {
	return renderModes.data();
}

void DeferredRenderer::SetRenderMode(uint16_t mode) {
	renderMode = static_cast<DeferredRenderMode>(mode);
}
