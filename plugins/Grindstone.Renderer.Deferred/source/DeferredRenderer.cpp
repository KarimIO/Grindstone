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

#include <Grindstone.Renderer.Deferred/include/DeferredRendererFactory.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRenderer.hpp>

using namespace Grindstone;

const size_t MAX_BLOOM_MIPS = 40u;
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
	deferredRendererImageSets.resize(maxFramesInFlight);

	bloomStoredMipLevelCount = bloomMipLevelCount = CalculateBloomLevels(renderArea.GetWidth(), renderArea.GetHeight());
	bloomFirstUpsampleIndex = bloomStoredMipLevelCount - 1;

	std::string_view iblBrdfLut = "@CORESHADERS/textures/ibl_brdf_lut";
	brdfLut = engineCore.assetManager->GetAssetReferenceByAddress<TextureAsset>(iblBrdfLut);

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

	Grindstone::GraphicsAPI::Sampler::CreateInfo screenSamplerCreateInfo{};
	screenSamplerCreateInfo.debugName = "Screen Sampler";
	screenSamplerCreateInfo.options.anistropy = 0;
	screenSamplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Linear;
	screenSamplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Linear;
	screenSamplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat;
	screenSamplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat;
	screenSamplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat;
	screenSampler = graphicsCore->CreateSampler(screenSamplerCreateInfo);

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		GraphicsAPI::Buffer::CreateInfo postProcessingUboCreateInfo{};
		postProcessingUboCreateInfo.debugName = "Post Processing UBO";
		postProcessingUboCreateInfo.content = &postProcessUboData;
		postProcessingUboCreateInfo.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform;
		postProcessingUboCreateInfo.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		postProcessingUboCreateInfo.bufferSize = sizeof(PostProcessUbo);

		deferredRendererImageSets[i].tonemapPostProcessingUniformBufferObject =
			graphicsCore->CreateBuffer(postProcessingUboCreateInfo);

		CreateDepthOfFieldRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateSsrRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateBloomRenderTargetsAndDescriptorSets(deferredRendererImageSets[i], i);
		CreateDescriptorSets(deferredRendererImageSets[i]);
	}

	CreatePipelines();
}

DeferredRenderer::~DeferredRenderer() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		DeferredRendererImageSet& imageSet = deferredRendererImageSets[i];

		graphicsCore->DeleteImage(imageSet.ssrRenderTarget);

		graphicsCore->DeleteBuffer(imageSet.debugUniformBufferObject);
		graphicsCore->DeleteBuffer(imageSet.tonemapPostProcessingUniformBufferObject);

		graphicsCore->DeleteDescriptorSet(imageSet.ssrDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.debugDescriptorSet);

		graphicsCore->DeleteDescriptorSet(imageSet.dofSourceDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.dofNearBlurDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.dofFarBlurDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.dofCombineDescriptorSet);

		graphicsCore->DeleteImage(imageSet.nearDofRenderTarget);
		graphicsCore->DeleteImage(imageSet.farDofRenderTarget);
		graphicsCore->DeleteImage(imageSet.nearBlurredDofRenderTarget);
		graphicsCore->DeleteImage(imageSet.farBlurredDofRenderTarget);

		for (GraphicsAPI::Image* bloomRt : imageSet.bloomRenderTargets) {
			graphicsCore->DeleteImage(bloomRt);
		}

		for (GraphicsAPI::DescriptorSet* bloomDs : imageSet.bloomDescriptorSets) {
			graphicsCore->DeleteDescriptorSet(bloomDs);
		}

		graphicsCore->DeleteBuffer(imageSet.globalUniformBufferObject);

		graphicsCore->DeleteImage(imageSet.gbufferAlbedoRenderTarget);
		graphicsCore->DeleteImage(imageSet.gbufferNormalRenderTarget);
		graphicsCore->DeleteImage(imageSet.gbufferSpecularRoughnessRenderTarget);

		graphicsCore->DeleteImage(imageSet.gbufferDepthStencilTarget);
		graphicsCore->DeleteImage(imageSet.litHdrRenderTarget);

		graphicsCore->DeleteDescriptorSet(imageSet.engineDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.tonemapDescriptorSet);
		graphicsCore->DeleteDescriptorSet(imageSet.gbufferDescriptorSet);

		graphicsCore->DeleteImage(imageSet.ambientOcclusionRenderTarget);
		graphicsCore->DeleteDescriptorSet(imageSet.ambientOcclusionDescriptorSet);

		graphicsCore->DeleteImage(imageSet.blurredAmbientOcclusionRenderTarget);
		graphicsCore->DeleteDescriptorSet(imageSet.blurredSsaoInputDescriptorSet);
	}

	for (GraphicsAPI::Buffer* bloomUb : bloomUniformBuffers) {
		graphicsCore->DeleteBuffer(bloomUb);
	}

	graphicsCore->DeleteDescriptorSetLayout(engineDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(tonemapDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(blurredSsaoInputDescriptorSetLayout);

	graphicsCore->DeleteBuffer(ssaoUniformBuffer);
	graphicsCore->DeleteImage(ssaoNoiseTexture);
	graphicsCore->DeleteDescriptorSet(ssaoInputDescriptorSet);
	graphicsCore->DeleteDescriptorSetLayout(ssaoInputDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(ambientOcclusionDescriptorSetLayout);

	graphicsCore->DeleteDescriptorSetLayout(bloomDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(ssrDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(debugDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingUBODescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(shadowMappedLightDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(lightingWithShadowUBODescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(shadowMapDescriptorSetLayout);

	graphicsCore->DeleteDescriptorSet(shadowMapDescriptorSet);

	graphicsCore->DeleteBuffer(vertexBuffer);
	graphicsCore->DeleteBuffer(indexBuffer);
	graphicsCore->DeleteVertexArrayObject(planePostProcessVao);

	graphicsCore->DeleteDescriptorSetLayout(dofSourceDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(dofBlurDescriptorSetLayout);
	graphicsCore->DeleteDescriptorSetLayout(dofCombinationDescriptorSetLayout);
}

void DeferredRenderer::CreateDepthOfFieldRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex) {
	EngineCore& engineCore = EngineCore::GetInstance();
	RenderPassRegistry* renderPassRegistry = engineCore.GetRenderPassRegistry();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	if (imageSet.nearDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.nearDofRenderTarget);
	}

	if (imageSet.farDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.farDofRenderTarget);
	}

	if (imageSet.nearBlurredDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.nearBlurredDofRenderTarget);
	}

	if (imageSet.farBlurredDofRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.farBlurredDofRenderTarget);
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
		GraphicsAPI::Image::CreateInfo dofRtCreateInfo{};
		dofRtCreateInfo.format = GraphicsAPI::Format::R16G16B16A16_SFLOAT;
		dofRtCreateInfo.width = framebufferWidth / 2;
		dofRtCreateInfo.height = framebufferHeight / 2;
		dofRtCreateInfo.imageUsage =
			GraphicsAPI::ImageUsageFlags::RenderTarget |
			GraphicsAPI::ImageUsageFlags::Sampled;

		dofRtCreateInfo.debugName = "Near DOF Render Target";
		imageSet.nearDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);
		dofRtCreateInfo.debugName = "Far DOF Render Target";
		imageSet.farDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);
		dofRtCreateInfo.debugName = "Near Blurred DOF Render Target";
		imageSet.nearBlurredDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);
		dofRtCreateInfo.debugName = "Far Blurred DOF Render Target";
		imageSet.farBlurredDofRenderTarget = graphicsCore->CreateImage(dofRtCreateInfo);

		imageSet.nearDofAttachment = {
			.image = imageSet.nearDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.farDofAttachment = {
			.image = imageSet.farDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.nearBlurredDofAttachment = {
			.image = imageSet.nearBlurredDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.farBlurredDofAttachment = {
			.image = imageSet.farBlurredDofRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> sourceDofDescriptorBindings = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferDepthStencilTarget ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget )
		};

		GraphicsAPI::DescriptorSet::CreateInfo dofSourceDescriptorSetCreateInfo{};
		dofSourceDescriptorSetCreateInfo.layout = dofSourceDescriptorSetLayout;
		dofSourceDescriptorSetCreateInfo.debugName = "Depth of Field Source Descriptor";
		dofSourceDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(sourceDofDescriptorBindings.size());
		dofSourceDescriptorSetCreateInfo.bindings = sourceDofDescriptorBindings.data();
		imageSet.dofSourceDescriptorSet = graphicsCore->CreateDescriptorSet(dofSourceDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSet::Binding nearDofDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.nearDofRenderTarget );

		GraphicsAPI::DescriptorSet::CreateInfo dofBlurNearDescriptorSetCreateInfo{};
		dofBlurNearDescriptorSetCreateInfo.layout = dofBlurDescriptorSetLayout;
		dofBlurNearDescriptorSetCreateInfo.debugName = "Depth of Field Blur Near Descriptor";
		dofBlurNearDescriptorSetCreateInfo.bindingCount = 1u;
		dofBlurNearDescriptorSetCreateInfo.bindings = &nearDofDescriptorBinding;
		imageSet.dofNearBlurDescriptorSet = graphicsCore->CreateDescriptorSet(dofBlurNearDescriptorSetCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSet::Binding farDofDescriptorBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.farDofRenderTarget );

		GraphicsAPI::DescriptorSet::CreateInfo dofBlurFarDescriptorSetCreateInfo{};
		dofBlurFarDescriptorSetCreateInfo.layout = dofBlurDescriptorSetLayout;
		dofBlurFarDescriptorSetCreateInfo.debugName = "Depth of Field Blur Far Descriptor";
		dofBlurFarDescriptorSetCreateInfo.bindingCount = 1u;
		dofBlurFarDescriptorSetCreateInfo.bindings = &farDofDescriptorBinding;
		imageSet.dofFarBlurDescriptorSet = graphicsCore->CreateDescriptorSet(dofBlurFarDescriptorSetCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> nearAndFarDescriptorBindings = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.nearBlurredDofRenderTarget ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.farBlurredDofRenderTarget )
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

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> sourceDofDescriptorBindings = {
			GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }, // Depth
			GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment } // Lit Scene
		};

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofSourceDescriptorSetLayoutCreateInfo{};
		dofSourceDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Source Descriptor Layout";
		dofSourceDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(sourceDofDescriptorBindings.size());
		dofSourceDescriptorSetLayoutCreateInfo.bindings = sourceDofDescriptorBindings.data();
		dofSourceDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(dofSourceDescriptorSetLayoutCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding farDofDescriptorBinding
			{ 0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment };

		GraphicsAPI::DescriptorSetLayout::CreateInfo dofBlurFarDescriptorSetLayoutCreateInfo{};
		dofBlurFarDescriptorSetLayoutCreateInfo.debugName = "Depth of Field Blur Far Descriptor Layout";
		dofBlurFarDescriptorSetLayoutCreateInfo.bindingCount = 1u;
		dofBlurFarDescriptorSetLayoutCreateInfo.bindings = &farDofDescriptorBinding;
		dofBlurDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(dofBlurFarDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> nearAndFarDescriptorBindings = {
			GraphicsAPI::DescriptorSetLayout::Binding{ 0, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }, // Near RT
			GraphicsAPI::DescriptorSetLayout::Binding{ 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment } // Far RT
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

	std::array<GraphicsAPI::DescriptorSetLayout::Binding, 5> bloomLayoutBindings{};
	bloomLayoutBindings[0].bindingId = 0;
	bloomLayoutBindings[0].count = 1;
	bloomLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
	bloomLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Compute;

	bloomLayoutBindings[1].bindingId = 1;
	bloomLayoutBindings[1].count = 1;
	bloomLayoutBindings[1].type = GraphicsAPI::BindingType::Sampler;
	bloomLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Compute;

	bloomLayoutBindings[2].bindingId = 2;
	bloomLayoutBindings[2].count = 1;
	bloomLayoutBindings[2].type = GraphicsAPI::BindingType::StorageImage;
	bloomLayoutBindings[2].stages = GraphicsAPI::ShaderStageBit::Compute;

	bloomLayoutBindings[3].bindingId = 3;
	bloomLayoutBindings[3].count = 1;
	bloomLayoutBindings[3].type = GraphicsAPI::BindingType::SampledImage;
	bloomLayoutBindings[3].stages = GraphicsAPI::ShaderStageBit::Compute;

	bloomLayoutBindings[4].bindingId = 4;
	bloomLayoutBindings[4].count = 1;
	bloomLayoutBindings[4].type = GraphicsAPI::BindingType::SampledImage;
	bloomLayoutBindings[4].stages = GraphicsAPI::ShaderStageBit::Compute;

	GraphicsAPI::DescriptorSetLayout::CreateInfo bloomDescriptorSetLayoutCreateInfo{};
	bloomDescriptorSetLayoutCreateInfo.debugName = "Bloom Descriptor Set Layout";
	bloomDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bloomLayoutBindings.size());
	bloomDescriptorSetLayoutCreateInfo.bindings = bloomLayoutBindings.data();
	bloomDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(bloomDescriptorSetLayoutCreateInfo);
}

void DeferredRenderer::CreateSSRResources() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::DescriptorSetLayout::Binding sourceBinding{};
	sourceBinding.count = 1;
	sourceBinding.type = GraphicsAPI::BindingType::SampledImage;
	sourceBinding.stages = GraphicsAPI::ShaderStageBit::Compute;

	std::array<GraphicsAPI::DescriptorSetLayout::Binding, 6> ssrLayoutBindings{};
	for (size_t i = 0; i < ssrLayoutBindings.size(); ++i) {
		ssrLayoutBindings[i] = sourceBinding;
		ssrLayoutBindings[i].bindingId = static_cast<uint32_t>(i);
	}

	ssrLayoutBindings[0].type = GraphicsAPI::BindingType::UniformBuffer;
	ssrLayoutBindings[1].type = GraphicsAPI::BindingType::StorageImage;
	ssrLayoutBindings[3].type = GraphicsAPI::BindingType::SampledImage;

	GraphicsAPI::DescriptorSetLayout::CreateInfo ssrDescriptorSetLayoutCreateInfo{};
	ssrDescriptorSetLayoutCreateInfo.debugName = "SSR Descriptor Set Layout";
	ssrDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssrLayoutBindings.size());
	ssrDescriptorSetLayoutCreateInfo.bindings = ssrLayoutBindings.data();
	ssrDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssrDescriptorSetLayoutCreateInfo);
}

void DeferredRenderer::CreateSsaoKernelAndNoise() {
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::uniform_int_distribution<short> randomShorts(0, std::numeric_limits<short>::max());
	std::default_random_engine generator;

	{
		SsaoUboStruct ssaoUboStruct{};
		ssaoUboStruct.bias = 0.025f;
		ssaoUboStruct.radius = 0.1f;

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

			float scale = static_cast<float>(i) / static_cast<float>(ssaoKernelSize);
			scale = 0.1f + (0.9f * scale * scale);
			sample *= scale;
			ssaoUboStruct.kernels[i] = sample;
		}

		GraphicsAPI::Buffer::CreateInfo ssaoUniformBufferObjectCi{};
		ssaoUniformBufferObjectCi.debugName = "SSAO Uniform Buffer";
		ssaoUniformBufferObjectCi.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform;
		ssaoUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
		ssaoUniformBufferObjectCi.bufferSize = sizeof(SsaoUboStruct);
		ssaoUniformBuffer = graphicsCore->CreateBuffer(ssaoUniformBufferObjectCi);
		ssaoUniformBuffer->UploadData(&ssaoUboStruct);
	}

	{
		constexpr size_t ssaoNoiseDimSize = 4;
		std::array<short, ssaoNoiseDimSize * ssaoNoiseDimSize> ssaoNoise{};
		for (size_t i = 0; i < ssaoNoise.size(); i++) {
			ssaoNoise[i] = randomShorts(generator);
		}

		GraphicsAPI::Image::CreateInfo ssaoNoiseImgCreateInfo{};
		ssaoNoiseImgCreateInfo.debugName = "SSAO Noise Texture";
		ssaoNoiseImgCreateInfo.initialData = reinterpret_cast<const char*>(ssaoNoise.data());
		ssaoNoiseImgCreateInfo.initialDataSize = static_cast<uint32_t>(sizeof(ssaoNoise));
		ssaoNoiseImgCreateInfo.format = GraphicsAPI::Format::R8G8_SNORM;
		ssaoNoiseImgCreateInfo.width = ssaoNoiseImgCreateInfo.height = ssaoNoiseDimSize;
		ssaoNoiseImgCreateInfo.imageUsage =
			GraphicsAPI::ImageUsageFlags::TransferDst |
			GraphicsAPI::ImageUsageFlags::TransferSrc |
			GraphicsAPI::ImageUsageFlags::Sampled;
		ssaoNoiseTexture = graphicsCore->CreateImage(ssaoNoiseImgCreateInfo);
	}

	{
		GraphicsAPI::Sampler::CreateInfo ssaoNoiseSamplerCreateInfo{};
		ssaoNoiseSamplerCreateInfo.debugName = "SSAO Noise Sampler";
		ssaoNoiseSamplerCreateInfo.options.magFilter = GraphicsAPI::TextureFilter::Nearest;
		ssaoNoiseSamplerCreateInfo.options.minFilter = GraphicsAPI::TextureFilter::Nearest;
		ssaoNoiseSamplerCreateInfo.options.wrapModeU = GraphicsAPI::TextureWrapMode::Repeat;
		ssaoNoiseSamplerCreateInfo.options.wrapModeV = GraphicsAPI::TextureWrapMode::Repeat;
		ssaoNoiseSamplerCreateInfo.options.wrapModeW = GraphicsAPI::TextureWrapMode::Repeat;
		ssaoNoiseSampler = graphicsCore->CreateSampler(ssaoNoiseSamplerCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 3> ssaoLayoutBindings{};
		ssaoLayoutBindings[0].bindingId = 0;
		ssaoLayoutBindings[0].count = 1;
		ssaoLayoutBindings[0].type = GraphicsAPI::BindingType::Sampler;
		ssaoLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoLayoutBindings[1].bindingId = 1;
		ssaoLayoutBindings[1].count = 1;
		ssaoLayoutBindings[1].type = GraphicsAPI::BindingType::SampledImage;
		ssaoLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoLayoutBindings[2].bindingId = 2;
		ssaoLayoutBindings[2].count = 1;
		ssaoLayoutBindings[2].type = GraphicsAPI::BindingType::UniformBuffer;
		ssaoLayoutBindings[2].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo ssaoDescriptorSetLayoutCreateInfo{};
		ssaoDescriptorSetLayoutCreateInfo.debugName = "SSAO Input Descriptor Set Layout";
		ssaoDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		ssaoDescriptorSetLayoutCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoInputDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssaoDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 3> ssaoLayoutBindings{
			GraphicsAPI::DescriptorSet::Binding::Sampler( ssaoNoiseSampler ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( ssaoNoiseTexture ),
			GraphicsAPI::DescriptorSet::Binding::UniformBuffer( ssaoUniformBuffer )
		};

		GraphicsAPI::DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
		engineDescriptorSetCreateInfo.debugName = "SSAO Input Descriptor Set";
		engineDescriptorSetCreateInfo.layout = ssaoInputDescriptorSetLayout;
		engineDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(ssaoLayoutBindings.size());
		engineDescriptorSetCreateInfo.bindings = ssaoLayoutBindings.data();
		ssaoInputDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);
	}
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

	this->renderArea = Grindstone::Math::IntRect2D(width, height);

	uint32_t halfWidth = width / 2u;
	uint32_t halfHeight = height / 2u;

	halfRenderArea = Grindstone::Math::IntRect2D(halfWidth, halfHeight);
	quarterRenderArea = Grindstone::Math::IntRect2D(halfWidth / 2u, halfHeight / 2u);

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

		imageSet.gbufferDepthStencilTarget->Resize(width, height);
		imageSet.litHdrRenderTarget->Resize(width, height);

		imageSet.ambientOcclusionRenderTarget->Resize(halfWidth, halfHeight);
		imageSet.blurredAmbientOcclusionRenderTarget->Resize(halfWidth, halfHeight);

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
			graphicsCore->DeleteBuffer(bloomUniformBuffers[i]);
		}
	}

	bloomUniformBuffers.resize(bloomStoredMipLevelCount * 2);

	GraphicsAPI::Buffer::CreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.debugName = "Bloom Uniform Buffer";
	uniformBufferCreateInfo.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Uniform;
	uniformBufferCreateInfo.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
	uniformBufferCreateInfo.bufferSize = sizeof(BloomUboStruct);

	for (size_t i = 0; i < bloomUniformBuffers.size(); ++i) {
		bloomUniformBuffers[i] = graphicsCore->CreateBuffer(uniformBufferCreateInfo);
	}
}

void DeferredRenderer::CreateSsrRenderTargetsAndDescriptorSets(DeferredRendererImageSet& imageSet, size_t imageSetIndex) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	if (imageSet.ssrRenderTarget != nullptr) {
		graphicsCore->DeleteImage(imageSet.ssrRenderTarget);
	}

	if (imageSet.ssrDescriptorSet != nullptr) {
		graphicsCore->DeleteDescriptorSet(imageSet.ssrDescriptorSet);
	}

	GraphicsAPI::Image::CreateInfo ssrRenderTargetCreateInfo{};
	ssrRenderTargetCreateInfo.debugName = "SSR Render Target";
	ssrRenderTargetCreateInfo.format = GraphicsAPI::Format::R16G16B16A16_SFLOAT;
	ssrRenderTargetCreateInfo.width = framebufferWidth;
	ssrRenderTargetCreateInfo.height = framebufferHeight;
	ssrRenderTargetCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::RenderTarget |
		GraphicsAPI::ImageUsageFlags::Storage |
		GraphicsAPI::ImageUsageFlags::Sampled;
	imageSet.ssrRenderTarget = graphicsCore->CreateImage(ssrRenderTargetCreateInfo);
	imageSet.ssrAttachment = {
		.image = imageSet.ssrRenderTarget,
		.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
		.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
		.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
		.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
	};

	std::array<GraphicsAPI::DescriptorSet::Binding, 6> descriptorBindings;
	descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( imageSet.globalUniformBufferObject );
	descriptorBindings[1] = GraphicsAPI::DescriptorSet::Binding::StorageImage( imageSet.ssrRenderTarget );
	descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget );
	descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferDepthStencilTarget );
	descriptorBindings[4] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferNormalRenderTarget );
	descriptorBindings[5] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferSpecularRoughnessRenderTarget );

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
			graphicsCore->DeleteImage(imageSet.bloomRenderTargets[i]);
		}
	}

	for (size_t i = 0; i < imageSet.bloomDescriptorSets.size(); ++i) {
		if (imageSet.bloomDescriptorSets[i] != nullptr) {
			graphicsCore->DeleteDescriptorSet(imageSet.bloomDescriptorSets[i]);
		}
	}

	imageSet.bloomRenderTargets.resize(bloomStoredMipLevelCount * 2);
	imageSet.bloomDescriptorSets.resize(bloomStoredMipLevelCount * 2 - 2);

	GraphicsAPI::Image::CreateInfo bloomRenderTargetCreateInfo{};
	bloomRenderTargetCreateInfo.debugName = "Bloom Render Target";
	bloomRenderTargetCreateInfo.format = GraphicsAPI::Format::R32G32B32A32_SFLOAT;
	bloomRenderTargetCreateInfo.width = framebufferWidth;
	bloomRenderTargetCreateInfo.height = framebufferHeight;
	bloomRenderTargetCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::RenderTarget |
		GraphicsAPI::ImageUsageFlags::Sampled |
		GraphicsAPI::ImageUsageFlags::Storage;

	for (uint32_t i = 0; i < bloomStoredMipLevelCount; ++i) {
		std::string bloomRenderTargetName = std::string("Bloom Render Target Downscale Mip ") + std::to_string(i);
		bloomRenderTargetCreateInfo.debugName = bloomRenderTargetName.c_str();
		imageSet.bloomRenderTargets[i] = graphicsCore->CreateImage(bloomRenderTargetCreateInfo);
		bloomRenderTargetCreateInfo.width = bloomRenderTargetCreateInfo.width / 2;
		bloomRenderTargetCreateInfo.height = bloomRenderTargetCreateInfo.height / 2;
	}

	bloomRenderTargetCreateInfo.width = framebufferWidth;
	bloomRenderTargetCreateInfo.height = framebufferHeight;

	for (uint32_t i = 0; i < bloomStoredMipLevelCount; ++i) {
		std::string bloomRenderTargetName = std::string("Bloom Render Target Upscale Mip ") + std::to_string(i);
		bloomRenderTargetCreateInfo.debugName = bloomRenderTargetName.c_str();
		imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i] = graphicsCore->CreateImage(bloomRenderTargetCreateInfo);
		bloomRenderTargetCreateInfo.width = bloomRenderTargetCreateInfo.width / 2;
		bloomRenderTargetCreateInfo.height = bloomRenderTargetCreateInfo.height / 2;
	}

	std::array<GraphicsAPI::DescriptorSet::Binding, 5> descriptorBindings;
	descriptorBindings[1] = GraphicsAPI::DescriptorSet::Binding::Sampler(screenSampler);
	descriptorBindings[4] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.bloomRenderTargets[0] );

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
		std::string bloomDescriptorName = std::vformat("Bloom DS Filter [{}]", std::make_format_args(imageSetIndex));
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UploadData(&bloomUboStruct);
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( bloomUniformBuffers[bloomDescriptorSetIndex] );
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage( imageSet.bloomRenderTargets[1] );
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget );
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Downsample;
	for (size_t i = 1; i < bloomStoredMipLevelCount - 1; ++i) {
		std::string bloomDescriptorName = std::vformat("Bloom DS Downsample [{}]({})", std::make_format_args(imageSetIndex, i));
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UploadData(&bloomUboStruct);
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( bloomUniformBuffers[bloomDescriptorSetIndex] );
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage( imageSet.bloomRenderTargets[i + 1] );
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.bloomRenderTargets[i] );
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	{
		std::string bloomDescriptorName = std::vformat("Bloom DS First Upsample [{}])", std::make_format_args(imageSetIndex));
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UploadData(&bloomUboStruct);
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(bloomUniformBuffers[bloomDescriptorSetIndex]);
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage(imageSet.bloomRenderTargets[bloomStoredMipLevelCount * 2 - 1]);
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[bloomStoredMipLevelCount - 2]);
		descriptorBindings[4] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[bloomStoredMipLevelCount - 1]);
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Upsample;
	for (size_t i = bloomStoredMipLevelCount - 2; i >= 1; --i) {
		std::string bloomDescriptorName = std::vformat("Bloom DS Upsample [{}]({})", std::make_format_args(imageSetIndex, i));
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UploadData(&bloomUboStruct);
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( bloomUniformBuffers[bloomDescriptorSetIndex] );
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage(imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i]);
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 1] );
		descriptorBindings[4] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.bloomRenderTargets[i] );
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void DeferredRenderer::UpdateBloomUBO() {
	std::vector<glm::vec2> mipSizes(bloomMipLevelCount);
	float mipWidth = static_cast<float>(renderArea.GetWidth());
	float mipHeight = static_cast<float>(renderArea.GetHeight());

	float widthMultiplier = static_cast<float>(mipWidth) / static_cast<float>(framebufferWidth);
	float heightMultiplier = static_cast<float>(mipHeight) / static_cast<float>(framebufferHeight);
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
		bloomUniformBuffers[bloomUboIndex++]->UploadData(&bloomUboStruct);

		bloomUboStruct.stage = BloomStage::Downsample;
		for (size_t i = 1; i < bloomMipLevelCount - 1; ++i) {
			bloomUboStruct.outReciprocalImgSize = mipSizes[i + 1];
			bloomUniformBuffers[bloomUboIndex++]->UploadData(&bloomUboStruct);
		}

		bloomUboStruct.stage = BloomStage::Upsample;
		{
			bloomUboStruct.outReciprocalImgSize = mipSizes[bloomMipLevelCount - 1];
			bloomUboStruct.inReciprocalImgSize = mipSizes[bloomMipLevelCount - 2];
			bloomUniformBuffers[bloomFirstUpsampleIndex]->UploadData(&bloomUboStruct);
		}

		bloomUboIndex = static_cast<uint32_t>((bloomStoredMipLevelCount * 2) - bloomMipLevelCount);
		for (size_t i = bloomMipLevelCount - 2; i >= 1; --i) {
			bloomUboStruct.outReciprocalImgSize = mipSizes[i];
			bloomUboStruct.inReciprocalImgSize = mipSizes[i - 1];
			bloomUniformBuffers[bloomUboIndex++]->UploadData(&bloomUboStruct);
		}

	}
}

void DeferredRenderer::UpdateBloomDescriptorSet(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	if (bloomMipLevelCount <= 2) {
		return;
	}

	std::array<GraphicsAPI::DescriptorSet::Binding, 3> bindings{
		GraphicsAPI::DescriptorSet::Binding::StorageImage(imageSet.bloomRenderTargets[(bloomStoredMipLevelCount * 2) - bloomMipLevelCount + 2]),
		GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[bloomMipLevelCount - 2]),
		GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[bloomMipLevelCount - 1])
	};

	imageSet.bloomDescriptorSets[bloomFirstUpsampleIndex]->ChangeBindings(bindings.data(), static_cast<uint32_t>(bindings.size()), 2);
}

void DeferredRenderer::CreateUniformBuffers() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::Buffer::CreateInfo globalUniformBufferObjectCi{};
	globalUniformBufferObjectCi.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Uniform;
	globalUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
	globalUniformBufferObjectCi.bufferSize = sizeof(EngineUboStruct);

	GraphicsAPI::Buffer::CreateInfo debugUniformBufferObjectCi{};
	debugUniformBufferObjectCi.bufferUsage = GraphicsAPI::BufferUsage::Uniform;
	debugUniformBufferObjectCi.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU;
	debugUniformBufferObjectCi.bufferSize = sizeof(DebugUboData);

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		std::string engineUboName = std::vformat("EngineUbo [{}]", std::make_format_args(i));
		std::string debugUboName = std::vformat("DebugUbo [{}]", std::make_format_args(i));

		globalUniformBufferObjectCi.debugName = engineUboName.c_str();
		debugUniformBufferObjectCi.debugName = debugUboName.c_str();

		DeferredRendererImageSet& imageSet = deferredRendererImageSets[i];
		imageSet.globalUniformBufferObject = graphicsCore->CreateBuffer(globalUniformBufferObjectCi);
		imageSet.debugUniformBufferObject = graphicsCore->CreateBuffer(debugUniformBufferObjectCi);
	}

	CreateDescriptorSetLayouts();
}

void DeferredRenderer::CreateDescriptorSetLayouts() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::DescriptorSetLayout::Binding engineUboBinding{};
	engineUboBinding.bindingId = 0;
	engineUboBinding.count = 1;
	engineUboBinding.type = GraphicsAPI::BindingType::UniformBuffer;
	engineUboBinding.stages = GraphicsAPI::ShaderStageBit::Vertex | GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::Binding gbufferSampler{};
	gbufferSampler.bindingId = 0;
	gbufferSampler.count = 1;
	gbufferSampler.type = GraphicsAPI::BindingType::Sampler;
	gbufferSampler.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::Binding litHdrRenderTargetBinding{};
	litHdrRenderTargetBinding.bindingId = 1;
	litHdrRenderTargetBinding.count = 1;
	litHdrRenderTargetBinding.type = GraphicsAPI::BindingType::SampledImage;
	litHdrRenderTargetBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::Binding gbufferDepthBinding{};
	gbufferDepthBinding.bindingId = 2;
	gbufferDepthBinding.count = 1;
	gbufferDepthBinding.type = GraphicsAPI::BindingType::SampledImage;
	gbufferDepthBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::Binding gbufferAlbedoBinding{};
	gbufferAlbedoBinding.bindingId = 3;
	gbufferAlbedoBinding.count = 1;
	gbufferAlbedoBinding.type = GraphicsAPI::BindingType::SampledImage;
	gbufferAlbedoBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::Binding gbufferNormalsBinding{};
	gbufferNormalsBinding.bindingId = 4;
	gbufferNormalsBinding.count = 1;
	gbufferNormalsBinding.type = GraphicsAPI::BindingType::SampledImage;
	gbufferNormalsBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	GraphicsAPI::DescriptorSetLayout::Binding gbufferSpecularRoughnessBinding{};
	gbufferSpecularRoughnessBinding.bindingId = 5;
	gbufferSpecularRoughnessBinding.count = 1;
	gbufferSpecularRoughnessBinding.type = GraphicsAPI::BindingType::SampledImage;
	gbufferSpecularRoughnessBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

	{
		GraphicsAPI::DescriptorSetLayout::CreateInfo engineDescriptorSetLayoutCreateInfo{};
		engineDescriptorSetLayoutCreateInfo.debugName = "Engine UBO Set Layout";
		engineDescriptorSetLayoutCreateInfo.bindingCount = 1;
		engineDescriptorSetLayoutCreateInfo.bindings = &engineUboBinding;
		engineDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(engineDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 4> tonemapDescriptorSetLayoutBindings{};
		tonemapDescriptorSetLayoutBindings[0] = gbufferSampler;
		tonemapDescriptorSetLayoutBindings[1] = litHdrRenderTargetBinding;
		tonemapDescriptorSetLayoutBindings[2] = { 2, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment };	// Bloom Texture
		tonemapDescriptorSetLayoutBindings[3] = { 3, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Fragment };	// Post Process Uniform Buffer

		GraphicsAPI::DescriptorSetLayout::CreateInfo tonemapDescriptorSetLayoutCreateInfo{};
		tonemapDescriptorSetLayoutCreateInfo.debugName = "Tonemap Descriptor Set Layout";
		tonemapDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetLayoutBindings.size());
		tonemapDescriptorSetLayoutCreateInfo.bindings = tonemapDescriptorSetLayoutBindings.data();
		tonemapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(tonemapDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 7> debugDescriptorSetLayoutBindings{};
		debugDescriptorSetLayoutBindings[0] = gbufferSampler;
		debugDescriptorSetLayoutBindings[1] = { 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Depth
		debugDescriptorSetLayoutBindings[2] = { 2, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Albedo
		debugDescriptorSetLayoutBindings[3] = { 3, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Normal
		debugDescriptorSetLayoutBindings[4] = { 4, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Specular Roughness
		debugDescriptorSetLayoutBindings[5] = { 5, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Ambient Occlusion
		debugDescriptorSetLayoutBindings[6] = { 6, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Fragment }; // Post Process Uniform Buffer

		GraphicsAPI::DescriptorSetLayout::CreateInfo debugDescriptorSetLayoutCreateInfo{};
		debugDescriptorSetLayoutCreateInfo.debugName = "Debug Descriptor Set Layout";
		debugDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(debugDescriptorSetLayoutBindings.size());
		debugDescriptorSetLayoutCreateInfo.bindings = debugDescriptorSetLayoutBindings.data();
		debugDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(debugDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 5> lightingDescriptorSetLayoutBindings{};
		lightingDescriptorSetLayoutBindings[0] = gbufferSampler;
		lightingDescriptorSetLayoutBindings[1] = { 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Depth
		lightingDescriptorSetLayoutBindings[2] = { 2, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Albedo
		lightingDescriptorSetLayoutBindings[3] = { 3, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Normal
		lightingDescriptorSetLayoutBindings[4] = { 4, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment }; // Gbuffer Specular Roughness

		GraphicsAPI::DescriptorSetLayout::CreateInfo lightingDescriptorSetLayoutCreateInfo{};
		lightingDescriptorSetLayoutCreateInfo.debugName = "GBuffer Descriptor Set Layout";
		lightingDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(lightingDescriptorSetLayoutBindings.size());
		lightingDescriptorSetLayoutCreateInfo.bindings = lightingDescriptorSetLayoutBindings.data();
		lightingDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(lightingDescriptorSetLayoutCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding ssaoGbufferNormalsBinding{};
		ssaoGbufferNormalsBinding.bindingId = 3;
		ssaoGbufferNormalsBinding.count = 1;
		ssaoGbufferNormalsBinding.type = GraphicsAPI::BindingType::SampledImage;
		ssaoGbufferNormalsBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 3> ssaoDescriptorSetLayoutBindings{};
		ssaoDescriptorSetLayoutBindings[0].bindingId = 0;
		ssaoDescriptorSetLayoutBindings[0].count = 1;
		ssaoDescriptorSetLayoutBindings[0].type = GraphicsAPI::BindingType::Sampler;
		ssaoDescriptorSetLayoutBindings[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoDescriptorSetLayoutBindings[1].bindingId = 1;
		ssaoDescriptorSetLayoutBindings[1].count = 1;
		ssaoDescriptorSetLayoutBindings[1].type = GraphicsAPI::BindingType::SampledImage;
		ssaoDescriptorSetLayoutBindings[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ssaoDescriptorSetLayoutBindings[2].bindingId = 2;
		ssaoDescriptorSetLayoutBindings[2].count = 1;
		ssaoDescriptorSetLayoutBindings[2].type = GraphicsAPI::BindingType::UniformBuffer;
		ssaoDescriptorSetLayoutBindings[2].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo ssaoDescriptorSetLayoutCreateInfo{};
		ssaoDescriptorSetLayoutCreateInfo.debugName = "SSAO Descriptor Set Layout";
		ssaoDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ssaoDescriptorSetLayoutBindings.size());
		ssaoDescriptorSetLayoutCreateInfo.bindings = ssaoDescriptorSetLayoutBindings.data();
		ssaoDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ssaoDescriptorSetLayoutCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding lightUboBinding{};
		lightUboBinding.bindingId = 0;
		lightUboBinding.count = 1;
		lightUboBinding.type = GraphicsAPI::BindingType::UniformBuffer;
		lightUboBinding.stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo lightingUBODescriptorSetLayoutCreateInfo{};
		lightingUBODescriptorSetLayoutCreateInfo.debugName = "Pointlight UBO Descriptor Set Layout";
		lightingUBODescriptorSetLayoutCreateInfo.bindingCount = 1;
		lightingUBODescriptorSetLayoutCreateInfo.bindings = &lightUboBinding;
		lightingUBODescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(lightingUBODescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> shadowMappedLightBindings{};
		shadowMappedLightBindings[0] = { 0, 1, GraphicsAPI::BindingType::UniformBuffer, GraphicsAPI::ShaderStageBit::Fragment };
		shadowMappedLightBindings[1] = { 1, 1, GraphicsAPI::BindingType::SampledImage, GraphicsAPI::ShaderStageBit::Fragment };

		GraphicsAPI::DescriptorSetLayout::CreateInfo shadowMappedLightDescriptorSetLayoutCreateInfo{};
		shadowMappedLightDescriptorSetLayoutCreateInfo.debugName = "Shadowmapped Light Descriptor Set Layout";
		shadowMappedLightDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(shadowMappedLightBindings.size());
		shadowMappedLightDescriptorSetLayoutCreateInfo.bindings = shadowMappedLightBindings.data();
		shadowMappedLightDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(shadowMappedLightDescriptorSetLayoutCreateInfo);
	}

	{
		GraphicsAPI::DescriptorSetLayout::Binding shadowMapMatrixBinding{};
		shadowMapMatrixBinding.bindingId = 0;
		shadowMapMatrixBinding.count = 1;
		shadowMapMatrixBinding.type = GraphicsAPI::BindingType::UniformBuffer;
		shadowMapMatrixBinding.stages = GraphicsAPI::ShaderStageBit::Vertex;

		GraphicsAPI::DescriptorSetLayout::CreateInfo shadowMapDescriptorSetLayoutCreateInfo{};
		shadowMapDescriptorSetLayoutCreateInfo.debugName = "Shadow Map Descriptor Set Layout";
		shadowMapDescriptorSetLayoutCreateInfo.bindingCount = 1;
		shadowMapDescriptorSetLayoutCreateInfo.bindings = &shadowMapMatrixBinding;
		shadowMapDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(shadowMapDescriptorSetLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 3> ambientOcclusionInputLayoutBinding{};
		ambientOcclusionInputLayoutBinding[0].bindingId = 0;
		ambientOcclusionInputLayoutBinding[0].count = 1;
		ambientOcclusionInputLayoutBinding[0].type = GraphicsAPI::BindingType::SampledImage;
		ambientOcclusionInputLayoutBinding[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ambientOcclusionInputLayoutBinding[1].bindingId = 1;
		ambientOcclusionInputLayoutBinding[1].count = 1;
		ambientOcclusionInputLayoutBinding[1].type = GraphicsAPI::BindingType::SampledImage;
		ambientOcclusionInputLayoutBinding[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ambientOcclusionInputLayoutBinding[2].bindingId = 2;
		ambientOcclusionInputLayoutBinding[2].count = 1;
		ambientOcclusionInputLayoutBinding[2].type = GraphicsAPI::BindingType::SampledImage;
		ambientOcclusionInputLayoutBinding[2].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo ambientOcclusionInputLayoutCreateInfo{};
		ambientOcclusionInputLayoutCreateInfo.debugName = "Ambient Occlusion Descriptor Set Layout";
		ambientOcclusionInputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ambientOcclusionInputLayoutBinding.size());
		ambientOcclusionInputLayoutCreateInfo.bindings = ambientOcclusionInputLayoutBinding.data();
		ambientOcclusionDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ambientOcclusionInputLayoutCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> ambientOcclusionInputBlurLayoutBinding{};
		ambientOcclusionInputBlurLayoutBinding[0].bindingId = 0;
		ambientOcclusionInputBlurLayoutBinding[0].count = 1;
		ambientOcclusionInputBlurLayoutBinding[0].type = GraphicsAPI::BindingType::Sampler;
		ambientOcclusionInputBlurLayoutBinding[0].stages = GraphicsAPI::ShaderStageBit::Fragment;

		ambientOcclusionInputBlurLayoutBinding[1].bindingId = 1;
		ambientOcclusionInputBlurLayoutBinding[1].count = 1;
		ambientOcclusionInputBlurLayoutBinding[1].type = GraphicsAPI::BindingType::SampledImage;
		ambientOcclusionInputBlurLayoutBinding[1].stages = GraphicsAPI::ShaderStageBit::Fragment;

		GraphicsAPI::DescriptorSetLayout::CreateInfo ambientOcclusionBlurInputLayoutCreateInfo{};
		ambientOcclusionBlurInputLayoutCreateInfo.debugName = "Ambient Occlusion Blur Descriptor Set Layout";
		ambientOcclusionBlurInputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(ambientOcclusionInputBlurLayoutBinding.size());
		ambientOcclusionBlurInputLayoutCreateInfo.bindings = ambientOcclusionInputBlurLayoutBinding.data();
		blurredSsaoInputDescriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ambientOcclusionBlurInputLayoutCreateInfo);
	}
}

void DeferredRenderer::CreateDescriptorSets(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	const GraphicsAPI::DescriptorSet::Binding screenSamplerBinding = GraphicsAPI::DescriptorSet::Binding::Sampler(screenSampler);
	const GraphicsAPI::DescriptorSet::Binding engineUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( imageSet.globalUniformBufferObject );
	const GraphicsAPI::DescriptorSet::Binding litHdrRenderTargetBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget );
	const GraphicsAPI::DescriptorSet::Binding gbufferDepthBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferDepthStencilTarget );
	const GraphicsAPI::DescriptorSet::Binding gbufferAlbedoBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferAlbedoRenderTarget );
	const GraphicsAPI::DescriptorSet::Binding gbufferNormalBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferNormalRenderTarget );
	const GraphicsAPI::DescriptorSet::Binding gbufferSpecRoughnessBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferSpecularRoughnessRenderTarget );

	GraphicsAPI::DescriptorSet::CreateInfo engineDescriptorSetCreateInfo{};
	engineDescriptorSetCreateInfo.debugName = "Engine UBO Descriptor Set";
	engineDescriptorSetCreateInfo.layout = engineDescriptorSetLayout;
	engineDescriptorSetCreateInfo.bindingCount = 1;
	engineDescriptorSetCreateInfo.bindings = &engineUboBinding;
	imageSet.engineDescriptorSet = graphicsCore->CreateDescriptorSet(engineDescriptorSetCreateInfo);

	std::array<GraphicsAPI::DescriptorSet::Binding, 4> tonemapDescriptorSetBindings{};
	tonemapDescriptorSetBindings[0] = GraphicsAPI::DescriptorSet::Binding::Sampler(screenSampler);
	tonemapDescriptorSetBindings[1] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget );
	tonemapDescriptorSetBindings[2] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.bloomRenderTargets[bloomMipLevelCount + 1] );
	tonemapDescriptorSetBindings[3] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( imageSet.tonemapPostProcessingUniformBufferObject );

	GraphicsAPI::DescriptorSet::CreateInfo tonemapDescriptorSetCreateInfo{};
	tonemapDescriptorSetCreateInfo.debugName = "Tonemap Descriptor Set";
	tonemapDescriptorSetCreateInfo.layout = tonemapDescriptorSetLayout;
	tonemapDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(tonemapDescriptorSetBindings.size());
	tonemapDescriptorSetCreateInfo.bindings = tonemapDescriptorSetBindings.data();
	imageSet.tonemapDescriptorSet = graphicsCore->CreateDescriptorSet(tonemapDescriptorSetCreateInfo);

	std::array<GraphicsAPI::DescriptorSet::Binding, 7> debugDescriptorSetBindings{};
	debugDescriptorSetBindings[0] = screenSamplerBinding;
	debugDescriptorSetBindings[1] = gbufferDepthBinding;
	debugDescriptorSetBindings[2] = gbufferAlbedoBinding;
	debugDescriptorSetBindings[3] = gbufferNormalBinding;
	debugDescriptorSetBindings[4] = gbufferSpecRoughnessBinding;
	debugDescriptorSetBindings[5] = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.blurredAmbientOcclusionRenderTarget );
	debugDescriptorSetBindings[6] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( imageSet.debugUniformBufferObject );

	GraphicsAPI::DescriptorSet::CreateInfo debugDescriptorSetCreateInfo{};
	debugDescriptorSetCreateInfo.debugName = "Debug Descriptor Set";
	debugDescriptorSetCreateInfo.layout = debugDescriptorSetLayout;
	debugDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(debugDescriptorSetBindings.size());
	debugDescriptorSetCreateInfo.bindings = debugDescriptorSetBindings.data();
	imageSet.debugDescriptorSet = graphicsCore->CreateDescriptorSet(debugDescriptorSetCreateInfo);

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 5> gbufferDescriptorSetBindings{};
		gbufferDescriptorSetBindings[0] = screenSamplerBinding;
		gbufferDescriptorSetBindings[1] = gbufferDepthBinding;
		gbufferDescriptorSetBindings[2] = gbufferAlbedoBinding;
		gbufferDescriptorSetBindings[3] = gbufferNormalBinding;
		gbufferDescriptorSetBindings[4] = gbufferSpecRoughnessBinding;

		GraphicsAPI::DescriptorSet::CreateInfo gbufferDescriptorSetCreateInfo{};
		gbufferDescriptorSetCreateInfo.debugName = "Gbuffer Descriptor Set";
		gbufferDescriptorSetCreateInfo.layout = lightingDescriptorSetLayout;
		gbufferDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(gbufferDescriptorSetBindings.size());
		gbufferDescriptorSetCreateInfo.bindings = gbufferDescriptorSetBindings.data();
		imageSet.gbufferDescriptorSet = graphicsCore->CreateDescriptorSet(gbufferDescriptorSetCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 3> gbufferDescriptorSetBindings{};
		gbufferDescriptorSetBindings[0] = screenSamplerBinding;
		gbufferDescriptorSetBindings[1] = gbufferDepthBinding;
		gbufferDescriptorSetBindings[2] = gbufferNormalBinding;

		GraphicsAPI::DescriptorSet::CreateInfo gbufferDescriptorSetCreateInfo{};
		gbufferDescriptorSetCreateInfo.debugName = "SSAO Gbuffer Descriptor Set";
		gbufferDescriptorSetCreateInfo.layout = lightingDescriptorSetLayout;
		gbufferDescriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(gbufferDescriptorSetBindings.size());
		gbufferDescriptorSetCreateInfo.bindings = gbufferDescriptorSetBindings.data();
		imageSet.ssaoGbufferDescriptorSet = graphicsCore->CreateDescriptorSet(gbufferDescriptorSetCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 3> aoInputBinding = {
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.blurredAmbientOcclusionRenderTarget ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( brdfLut.Get()->image ),
			GraphicsAPI::DescriptorSet::Binding::SampledImage( nullptr )
		};

		GraphicsAPI::DescriptorSet::CreateInfo aoInputCreateInfo{};
		aoInputCreateInfo.debugName = "Ambient Occlusion Descriptor Set";
		aoInputCreateInfo.layout = ambientOcclusionDescriptorSetLayout;
		aoInputCreateInfo.bindingCount = static_cast<uint32_t>(aoInputBinding.size());
		aoInputCreateInfo.bindings = aoInputBinding.data();
		imageSet.ambientOcclusionDescriptorSet = graphicsCore->CreateDescriptorSet(aoInputCreateInfo);
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 2> aoBlurInputBinding = {
			screenSamplerBinding,
			GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.ambientOcclusionRenderTarget )
		};

		GraphicsAPI::DescriptorSet::CreateInfo aoBlurInputCreateInfo{};
		aoBlurInputCreateInfo.debugName = "Ambient Occlusion Blur Descriptor Set";
		aoBlurInputCreateInfo.layout = blurredSsaoInputDescriptorSetLayout;
		aoBlurInputCreateInfo.bindingCount = static_cast<uint32_t>(aoBlurInputBinding.size());
		aoBlurInputCreateInfo.bindings = aoBlurInputBinding.data();
		imageSet.blurredSsaoInputDescriptorSet = graphicsCore->CreateDescriptorSet(aoBlurInputCreateInfo);
	}
}

void DeferredRenderer::UpdateDescriptorSets(DeferredRendererImageSet& imageSet) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	GraphicsAPI::DescriptorSet::Binding engineUboBinding = GraphicsAPI::DescriptorSet::Binding::UniformBuffer( imageSet.globalUniformBufferObject );
	GraphicsAPI::DescriptorSet::Binding screenSamplerBinding = GraphicsAPI::DescriptorSet::Binding::Sampler( screenSampler );
	GraphicsAPI::DescriptorSet::Binding litHdrRenderTargetBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.litHdrRenderTarget );
	GraphicsAPI::DescriptorSet::Binding gbufferDepthBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferDepthStencilTarget );
	GraphicsAPI::DescriptorSet::Binding gbufferAlbedoBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferAlbedoRenderTarget );
	GraphicsAPI::DescriptorSet::Binding gbufferNormalBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferNormalRenderTarget );
	GraphicsAPI::DescriptorSet::Binding gbufferSpecRoughnessBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage( imageSet.gbufferSpecularRoughnessRenderTarget );
	imageSet.engineDescriptorSet->ChangeBindings(&engineUboBinding, 1);

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 4> tonemapDescriptorSetBindings{};
		tonemapDescriptorSetBindings[0] = GraphicsAPI::DescriptorSet::Binding::Sampler(screenSampler);
		tonemapDescriptorSetBindings[1] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.litHdrRenderTarget);
		tonemapDescriptorSetBindings[2] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[bloomMipLevelCount + 1]);
		tonemapDescriptorSetBindings[3] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(imageSet.tonemapPostProcessingUniformBufferObject);

		imageSet.tonemapDescriptorSet->ChangeBindings(tonemapDescriptorSetBindings.data(), static_cast<uint32_t>(tonemapDescriptorSetBindings.size()));
	}

	{
		std::array<GraphicsAPI::DescriptorSet::Binding, 5> gbufferDescriptorSetBindings{};
		gbufferDescriptorSetBindings[0] = screenSamplerBinding;
		gbufferDescriptorSetBindings[1] = gbufferDepthBinding;
		gbufferDescriptorSetBindings[2] = gbufferAlbedoBinding;
		gbufferDescriptorSetBindings[3] = gbufferNormalBinding;
		gbufferDescriptorSetBindings[4] = gbufferSpecRoughnessBinding;

		imageSet.gbufferDescriptorSet->ChangeBindings(gbufferDescriptorSetBindings.data(), static_cast<uint32_t>(gbufferDescriptorSetBindings.size()));
	}

	{
		GraphicsAPI::DescriptorSet::Binding ssaoInputBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.blurredAmbientOcclusionRenderTarget);
		imageSet.ambientOcclusionDescriptorSet->ChangeBindings(&ssaoInputBinding, 1, 0);
	}

	{
		GraphicsAPI::DescriptorSet::Binding blurredSsaoInputBinding = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.ambientOcclusionRenderTarget);
		imageSet.blurredSsaoInputDescriptorSet->ChangeBindings(&blurredSsaoInputBinding, 1, 1);
	}
}

void DeferredRenderer::CreateVertexAndIndexBuffersAndLayouts() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	vertexLightPositionLayout = GraphicsAPI::VertexInputLayoutBuilder().AddBinding(
		{ 0, 2 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex},
		{
			{
				"vertexPosition",
				0,
				Grindstone::GraphicsAPI::Format::R32G32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		}
	).Build();

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

	GraphicsAPI::VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = "Light Vertex Array Object";
	vaoCi.vertexBufferCount = 1;
	vaoCi.vertexBuffers = &vertexBuffer;
	vaoCi.indexBuffer = indexBuffer;
	vaoCi.layout = vertexLightPositionLayout;
	planePostProcessVao = graphicsCore->CreateVertexArrayObject(vaoCi);
}

void DeferredRenderer::CreateGbufferFramebuffer() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	const int gbufferColorCount = 3;
	std::array<GraphicsAPI::RenderPass::AttachmentInfo, gbufferColorCount> gbufferColorAttachments{};
	gbufferColorAttachments[0] = { GraphicsAPI::Format::R8G8B8A8_UNORM, true }; // Albedo
	gbufferColorAttachments[1] = { GraphicsAPI::Format::R16G16B16A16_SNORM, true }; // Normal
	gbufferColorAttachments[2] = { GraphicsAPI::Format::R8G8B8A8_UNORM, true }; // Specular RGB + Roughness Alpha

	std::array<GraphicsAPI::Image*, gbufferColorCount> gbufferRenderTargets{};
	GraphicsAPI::Image::CreateInfo gbufferDepthImageCreateInfo{};
	gbufferDepthImageCreateInfo.debugName = "GBuffer Depth Image";
	gbufferDepthImageCreateInfo.format = depthFormat;
	gbufferDepthImageCreateInfo.width = framebufferWidth;
	gbufferDepthImageCreateInfo.height = framebufferHeight;
	gbufferDepthImageCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::TransferSrc |
		GraphicsAPI::ImageUsageFlags::DepthStencil |
		GraphicsAPI::ImageUsageFlags::Sampled;

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		{
			GraphicsAPI::Image::CreateInfo gbufferRtCreateInfo{};
			gbufferRtCreateInfo.format = gbufferColorAttachments[0].colorFormat;
			gbufferRtCreateInfo.debugName = "GBuffer Albedo Image";
			gbufferRtCreateInfo.width = framebufferWidth;
			gbufferRtCreateInfo.height = framebufferHeight;
			gbufferRtCreateInfo.imageUsage =
				GraphicsAPI::ImageUsageFlags::RenderTarget |
				GraphicsAPI::ImageUsageFlags::Sampled;

			gbufferRenderTargets[0] = imageSet.gbufferAlbedoRenderTarget = graphicsCore->CreateImage(gbufferRtCreateInfo);

			gbufferRtCreateInfo.format = gbufferColorAttachments[1].colorFormat;
			gbufferRtCreateInfo.debugName = "GBuffer Normal Image";

			gbufferRenderTargets[1] = imageSet.gbufferNormalRenderTarget = graphicsCore->CreateImage(gbufferRtCreateInfo);

			gbufferRtCreateInfo.format = gbufferColorAttachments[2].colorFormat;
			gbufferRtCreateInfo.debugName = "GBuffer Specular + Roughness Image";

			gbufferRenderTargets[2] = imageSet.gbufferSpecularRoughnessRenderTarget = graphicsCore->CreateImage(gbufferRtCreateInfo);
		}

		imageSet.gbufferAlbedoAttachment = {
			.image = imageSet.gbufferAlbedoRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};
		imageSet.gbufferNormalAttachment = {
			.image = imageSet.gbufferNormalRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};
		imageSet.gbufferSpecularRoughnessAttachment = {
			.image = imageSet.gbufferSpecularRoughnessRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};

		imageSet.gbufferDepthStencilTarget = graphicsCore->CreateImage(gbufferDepthImageCreateInfo);
		imageSet.gbufferDepthStencilAttachment = {
			.image = imageSet.gbufferDepthStencilTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u),
		};

		imageSet.forwardDepthStencilAttachment = {
			.image = imageSet.gbufferDepthStencilTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Load,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearDepthStencil(1.0f, 0u),
		};

		{
			GraphicsAPI::Image::CreateInfo ssaoRenderTargetCreateInfo{};
			ssaoRenderTargetCreateInfo.debugName = "SSAO Render Target";
			ssaoRenderTargetCreateInfo.format = ambientOcclusionFormat;
			ssaoRenderTargetCreateInfo.width = framebufferWidth / 2;
			ssaoRenderTargetCreateInfo.height = framebufferHeight / 2;
			ssaoRenderTargetCreateInfo.imageUsage =
				GraphicsAPI::ImageUsageFlags::RenderTarget |
				GraphicsAPI::ImageUsageFlags::Sampled;

			imageSet.ambientOcclusionRenderTarget = graphicsCore->CreateImage(ssaoRenderTargetCreateInfo);
			imageSet.ambientOcclusionAttachment = {
				.image = imageSet.ambientOcclusionRenderTarget,
				.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
			};
		}

		{
			GraphicsAPI::Image::CreateInfo ssaoRenderTargetCreateInfo{};
			ssaoRenderTargetCreateInfo.debugName = "Blurred SSAO Render Target";
			ssaoRenderTargetCreateInfo.format = ambientOcclusionFormat;
			ssaoRenderTargetCreateInfo.width = framebufferWidth / 2;
			ssaoRenderTargetCreateInfo.height = framebufferHeight / 2;
			ssaoRenderTargetCreateInfo.imageUsage =
				GraphicsAPI::ImageUsageFlags::RenderTarget |
				GraphicsAPI::ImageUsageFlags::Sampled;

			imageSet.blurredAmbientOcclusionRenderTarget = graphicsCore->CreateImage(ssaoRenderTargetCreateInfo);
			imageSet.blurredAmbientOcclusionAttachment = {
				.image = imageSet.blurredAmbientOcclusionRenderTarget,
				.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
			};
		}
	}
}

void DeferredRenderer::CreateLitHDRFramebuffer() {
	Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::Image::CreateInfo litHdrImagesCreateInfo{};
	litHdrImagesCreateInfo.width = framebufferWidth;
	litHdrImagesCreateInfo.height = framebufferHeight;
	litHdrImagesCreateInfo.format = Grindstone::GraphicsAPI::Format::R16G16B16A16_SFLOAT;
	litHdrImagesCreateInfo.imageUsage =
		GraphicsAPI::ImageUsageFlags::RenderTarget |
		GraphicsAPI::ImageUsageFlags::Sampled;

	GraphicsAPI::RenderPass::AttachmentInfo attachment{ litHdrImagesCreateInfo.format , true };

	for (size_t i = 0; i < deferredRendererImageSets.size(); ++i) {
		auto& imageSet = deferredRendererImageSets[i];

		std::string rtName = "Lit HDR Color Image [" + std::to_string(i) + "]";
		litHdrImagesCreateInfo.debugName = rtName.c_str();

		imageSet.litHdrRenderTarget = graphicsCore->CreateImage(litHdrImagesCreateInfo);
		imageSet.litHdrAttachment = {
			.image = imageSet.litHdrRenderTarget,
			.imageLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
			.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
			.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
			.clearValue = Grindstone::GraphicsAPI::ClearColor(0.0f, 0.0f, 0.0f, 0.0f),
		};
	}
}

void DeferredRenderer::CreatePipelines() {
	Grindstone::Assets::AssetManager* assetManager = EngineCore::GetInstance().assetManager;

	bloomPipelineSet = assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/bloom");
	ssrPipelineSet = assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/screenSpaceReflections");

	ssaoPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/screenSpaceAmbientOcclusion");
	ssaoBlurPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/screenSpaceAmbientOcclusionBlur");
	imageBasedLightingPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/ibl");
	pointLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/point");
	spotLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/spot");
	directionalLightPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/lighting/directional");
	debugPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/editor/debug");
	tonemapPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/tonemapping");
	dofSeparationPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/dofSeparation");
	dofBlurPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/dofBlur");
	dofCombinationPipelineSet = assetManager->GetAssetReferenceByAddress<GraphicsPipelineAsset>("@CORESHADERS/postProcessing/dofCombination");
}

void DeferredRenderer::RenderDepthOfField(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::ClearColor singleClearColor{ 0.0f, 0.0f, 0.0f, 0.f };

	GraphicsAPI::ClearDepthStencil depthStencilClear;

	currentCommandBuffer->BeginDebugLabelSection("Depth of Field Pass", nullptr);

	{
		std::array<Grindstone::GraphicsAPI::RenderAttachment, 2> attachments = {
			imageSet.nearDofAttachment,
			imageSet.farDofAttachment
		};
		currentCommandBuffer->BeginRendering(
			"DOF Separation Pass (Depth of Field)",
			halfRenderArea,
			attachments.data(),
			static_cast<uint32_t>(attachments.size()),
			nullptr,
			nullptr
		);

		Grindstone::GraphicsAPI::GraphicsPipeline* dofSeparationPipeline = dofSeparationPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);
		std::array<GraphicsAPI::DescriptorSet*, 2> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofSourceDescriptorSet
		};
		currentCommandBuffer->BindGraphicsPipeline(dofSeparationPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofSeparationPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* dofBlurPipeline = dofBlurPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);

	{
		currentCommandBuffer->BeginRendering(
			"DOF Near Blur Pass (Depth of Field)",
			quarterRenderArea,
			&imageSet.nearBlurredDofAttachment,
			1u,
			nullptr,
			nullptr
		);


		std::array<GraphicsAPI::DescriptorSet*, 2> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofNearBlurDescriptorSet
		};

		currentCommandBuffer->BindGraphicsPipeline(dofBlurPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofBlurPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	{
		currentCommandBuffer->BeginRendering(
			"DOF Far Blur Pass (Depth of Field)",
			quarterRenderArea,
			&imageSet.farBlurredDofAttachment,
			1u,
			nullptr,
			nullptr
		);

		std::array<GraphicsAPI::DescriptorSet*, 2> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofFarBlurDescriptorSet
		};

		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofBlurPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	{
		currentCommandBuffer->BeginRendering(
			"DOF Combination Pass (Depth of Field)",
			renderArea,
			&imageSet.litHdrAttachment,
			1u,
			nullptr,
			nullptr
		);

		std::array<GraphicsAPI::DescriptorSet*, 3> descriptorSets = {
			imageSet.engineDescriptorSet,
			imageSet.dofSourceDescriptorSet,
			imageSet.dofCombineDescriptorSet
		};
		Grindstone::GraphicsAPI::GraphicsPipeline* dofCombinationPipeline = dofCombinationPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);
		currentCommandBuffer->BindGraphicsPipeline(dofCombinationPipeline);
		currentCommandBuffer->BindGraphicsDescriptorSet(
			dofCombinationPipeline,
			descriptorSets.data(),
			0,
			static_cast<uint32_t>(descriptorSets.size())
		);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		currentCommandBuffer->EndRendering();
	}

	currentCommandBuffer->EndDebugLabelSection();
}

void DeferredRenderer::RenderSsr(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	Grindstone::ComputePipelineAsset* ssrPipelineAsset = ssrPipelineSet.Get();
	if (ssrPipelineAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::ComputePipeline* ssrPipeline = ssrPipelineAsset->GetPipeline();
	if (ssrPipeline == nullptr) {
		return;
	}

	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	currentCommandBuffer->BeginDebugLabelSection("Screen Space Reflections Pass", nullptr);
	currentCommandBuffer->BindComputePipeline(ssrPipeline);

	{
		GraphicsAPI::ImageBarrier barrier{
			.image = imageSet.ssrRenderTarget,
			.oldLayout = GraphicsAPI::ImageLayout::ShaderRead,
			.newLayout = GraphicsAPI::ImageLayout::General,
			.srcAccess = GraphicsAPI::AccessFlags::ShaderRead,
			.dstAccess = GraphicsAPI::AccessFlags::ShaderWrite,
			.imageAspect = GraphicsAPI::ImageAspectBits::Color,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&barrier, 1
		);
		currentCommandBuffer->BindComputeDescriptorSet(ssrPipeline, &imageSet.ssrDescriptorSet, 2, 1);
		currentCommandBuffer->DispatchCompute(renderArea.GetWidth(), renderArea.GetHeight(), 1);

		barrier.image = imageSet.ssrRenderTarget;
		barrier.oldLayout = GraphicsAPI::ImageLayout::General;
		barrier.newLayout = GraphicsAPI::ImageLayout::ShaderRead;
		barrier.srcAccess = GraphicsAPI::AccessFlags::ShaderWrite;
		barrier.dstAccess = GraphicsAPI::AccessFlags::ShaderRead;

		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&barrier, 1
		);
	}
	currentCommandBuffer->EndDebugLabelSection();
}

void DeferredRenderer::RenderBloom(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* currentCommandBuffer) {
	Grindstone::ComputePipelineAsset* bloomPipelineAsset = bloomPipelineSet.Get();
	if (bloomPipelineAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline = bloomPipelineAsset->GetPipeline();
	if (bloomPipeline == nullptr || bloomMipLevelCount <= 2) {
		return;
	}

	static float debugColor[4] = { 1.0f, 0.6f, 0.55f, 1.0f };
	static float debugColorLevel2[4] = { 0.9f, 0.5f, 0.4f, 1.0f };

	currentCommandBuffer->BeginDebugLabelSection("Bloom Pass", debugColor);
	GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	currentCommandBuffer->BindComputePipeline(bloomPipeline);
	uint32_t groupCountX = static_cast<uint32_t>(std::ceil(quarterRenderArea.GetWidth()));
	uint32_t groupCountY = static_cast<uint32_t>(std::ceil(quarterRenderArea.GetHeight()));
	uint32_t descriptorSetIndex = 0;

	GraphicsAPI::ImageBarrier readImageBarrier{
		.oldLayout = GraphicsAPI::ImageLayout::General,
		.newLayout = GraphicsAPI::ImageLayout::ShaderRead,
		.srcAccess = GraphicsAPI::AccessFlags::ShaderWrite,
		.dstAccess = GraphicsAPI::AccessFlags::ShaderRead,
		.imageAspect = GraphicsAPI::ImageAspectBits::Color,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	GraphicsAPI::ImageBarrier writeImageBarrier{
		.oldLayout = GraphicsAPI::ImageLayout::ShaderRead,
		.newLayout = GraphicsAPI::ImageLayout::General,
		.srcAccess = GraphicsAPI::AccessFlags::ShaderRead,
		.dstAccess = GraphicsAPI::AccessFlags::ShaderWrite,
		.imageAspect = GraphicsAPI::ImageAspectBits::Color,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	currentCommandBuffer->BeginDebugLabelSection("Bloom First Downsample", debugColor);
	{
		writeImageBarrier.image = imageSet.bloomRenderTargets[1];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&writeImageBarrier, 1
		);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 2, 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}
	currentCommandBuffer->EndDebugLabelSection();

	uint32_t mipWidth = static_cast<uint32_t>(renderArea.GetWidth());
	uint32_t mipHeight = static_cast<uint32_t>(renderArea.GetHeight());

	std::vector<uint32_t> mipWidths(bloomMipLevelCount);
	std::vector<uint32_t> mipHeights(bloomMipLevelCount);
	mipWidths[0] = mipWidth;
	mipHeights[0] = mipHeight;

	currentCommandBuffer->BeginDebugLabelSection("Bloom Downsamples", debugColor);
	for (size_t i = 1; i < bloomMipLevelCount - 1; ++i) {
		mipWidth = static_cast<uint32_t>(glm::ceil(static_cast<float>(mipWidth) / 2.0f));
		mipHeight = static_cast<uint32_t>(glm::ceil(static_cast<float>(mipHeight) / 2.0f));
		mipWidths[i] = mipWidth;
		mipHeights[i] = mipHeight;

		groupCountX = static_cast<uint32_t>(std::ceil(mipWidth / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeight / 4.0f));

		readImageBarrier.image = imageSet.bloomRenderTargets[i];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&readImageBarrier, 1
		);

		writeImageBarrier.image = imageSet.bloomRenderTargets[i + 1];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&writeImageBarrier, 1
		);
		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 2, 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}
	currentCommandBuffer->EndDebugLabelSection();

	currentCommandBuffer->BeginDebugLabelSection("Bloom First Upsample", debugColor);
	{
		groupCountX = static_cast<uint32_t>(std::ceil(mipWidths[bloomMipLevelCount - 2] / 4.0f));
		groupCountY = static_cast<uint32_t>(std::ceil(mipHeights[bloomMipLevelCount - 2] / 4.0f));

		readImageBarrier.image = imageSet.bloomRenderTargets[bloomMipLevelCount - 1];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&readImageBarrier, 1
		);

		writeImageBarrier.image = imageSet.bloomRenderTargets[(bloomStoredMipLevelCount * 2) - bloomMipLevelCount + 2];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&writeImageBarrier, 1
		);

		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[bloomFirstUpsampleIndex], 2, 1);
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

		writeImageBarrier.image = imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 1];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&writeImageBarrier, 1
		);

		readImageBarrier.image = imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 2];
		currentCommandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ComputeShader,
			GraphicsAPI::PipelineStageBit::ComputeShader,
			nullptr, 0,
			&readImageBarrier, 1
		);

		currentCommandBuffer->BindComputeDescriptorSet(bloomPipeline, &imageSet.bloomDescriptorSets[descriptorSetIndex++], 2, 1);
		currentCommandBuffer->DispatchCompute(groupCountX, groupCountY, 1);
	}
	currentCommandBuffer->EndDebugLabelSection();

	currentCommandBuffer->EndDebugLabelSection();

	readImageBarrier.image = imageSet.bloomRenderTargets[bloomStoredMipLevelCount + 1];
	currentCommandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::ComputeShader,
		GraphicsAPI::PipelineStageBit::ComputeShader,
		nullptr, 0,
		&readImageBarrier, 1
	);
}

void DeferredRenderer::RenderLights(
	uint32_t imageIndex,
	GraphicsAPI::CommandBuffer* currentCommandBuffer,
	entt::registry& registry
) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::DeferredRenderer::DeferredRendererImageSet& imageSet = deferredRendererImageSets[imageIndex];
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;

	const glm::mat4 bias = glm::mat4( 
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);

	Grindstone::GraphicsPipelineAsset* imageBasedLightingAsset = imageBasedLightingPipelineSet.Get();
	if (imageBasedLightingAsset != nullptr) {
		Grindstone::GraphicsAPI::GraphicsPipeline* imageBasedLightingPipeline = imageBasedLightingAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (imageBasedLightingPipeline != nullptr) {
			currentCommandBuffer->BeginDebugLabelSection("Image Based Lighting", nullptr);
			currentCommandBuffer->BindGraphicsPipeline(imageBasedLightingPipeline);

			auto view = registry.view<const EnvironmentMapComponent>();

			bool hasEnvMap = false;
			view.each([&imageSet, &hasEnvMap](const EnvironmentMapComponent& environmentMapComponent) {
				// Valid env map found - ignore ther est.
				if (hasEnvMap) {
					return;
				}

				const Grindstone::TextureAsset* texAsset = environmentMapComponent.specularTexture.Get();
				// Invalid env map - keep searching.
				if (texAsset == nullptr || texAsset->image == nullptr) {
					return;
				}

				hasEnvMap = true;
				Grindstone::GraphicsAPI::Image* image = texAsset->image;
				if (imageSet.currentEnvironmentMapImage == image) {
					// We found the correct env map and we're already using it - just send it forward to render.
					return;
				}

				// We found the correct env map and it is different from the current one - change it and then render.
				GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::SampledImage(image);
				imageSet.ambientOcclusionDescriptorSet->ChangeBindings(&binding, 1, 2);
				imageSet.currentEnvironmentMapImage = image;
			});

			if (hasEnvMap) {
				std::array<GraphicsAPI::DescriptorSet*, 3> iblDescriptors{};
				iblDescriptors[0] = imageSet.engineDescriptorSet;
				iblDescriptors[1] = imageSet.gbufferDescriptorSet;
				iblDescriptors[2] = imageSet.ambientOcclusionDescriptorSet;
				currentCommandBuffer->BindGraphicsDescriptorSet(imageBasedLightingPipeline, iblDescriptors.data(), 0, static_cast<uint32_t>(iblDescriptors.size()));
				currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
			}

			currentCommandBuffer->EndDebugLabelSection();
		}
	}

	Grindstone::GraphicsPipelineAsset* pointLightAsset = pointLightPipelineSet.Get();
	if (pointLightAsset != nullptr) {
		Grindstone::GraphicsAPI::GraphicsPipeline* pointLightPipeline = pointLightAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (pointLightPipeline != nullptr) {
			currentCommandBuffer->BeginDebugLabelSection("Point Lighting", nullptr);
			currentCommandBuffer->BindGraphicsPipeline(pointLightPipeline);

			std::array<GraphicsAPI::DescriptorSet*, 2> pointLightDescriptors{};
			pointLightDescriptors[0] = imageSet.gbufferDescriptorSet;

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
				pointLightComponent.uniformBufferObject->UploadData(&lightmapStruct);
				currentCommandBuffer->BindGraphicsDescriptorSet(pointLightPipeline, pointLightDescriptors.data(), 1, static_cast<uint32_t>(pointLightDescriptors.size()));
				currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
			});
			currentCommandBuffer->EndDebugLabelSection();
		}
	}

	Grindstone::GraphicsPipelineAsset* spotLightAsset = spotLightPipelineSet.Get();
	if (spotLightAsset != nullptr) {
		Grindstone::GraphicsAPI::GraphicsPipeline* spotLightPipeline = spotLightAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (spotLightPipeline != nullptr) {
			currentCommandBuffer->BeginDebugLabelSection("Spot Lighting", nullptr);
			currentCommandBuffer->BindGraphicsPipeline(spotLightPipeline);

			std::array<GraphicsAPI::DescriptorSet*, 2> spotLightDescriptors{};
			spotLightDescriptors[0] = imageSet.gbufferDescriptorSet;

			auto view = registry.view<const entt::entity, SpotLightComponent>();
			view.each([&](const entt::entity entityHandle, SpotLightComponent& spotLightComponent) {
				const ECS::Entity entity(entityHandle, scene);

				SpotLightComponent::UniformStruct lightStruct{
					bias * spotLightComponent.shadowMatrix,
					spotLightComponent.color,
					spotLightComponent.attenuationRadius,
					entity.GetWorldPosition(),
					spotLightComponent.intensity,
					entity.GetWorldForward(),
					glm::cos(glm::radians(spotLightComponent.innerAngle)),
					glm::cos(glm::radians(spotLightComponent.outerAngle)),
					static_cast<float>(spotLightComponent.shadowResolution)
				};

				spotLightComponent.uniformBufferObject->UploadData(&lightStruct);

				spotLightDescriptors[1] = spotLightComponent.descriptorSet;
				currentCommandBuffer->BindGraphicsDescriptorSet(spotLightPipeline, spotLightDescriptors.data(), 1, static_cast<uint32_t>(spotLightDescriptors.size()));
				currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
			});
			currentCommandBuffer->EndDebugLabelSection();
		}
	}

	Grindstone::GraphicsPipelineAsset* directionalLightAsset = directionalLightPipelineSet.Get();
	if (directionalLightAsset != nullptr) {
		Grindstone::GraphicsAPI::GraphicsPipeline* directionalLightPipeline = directionalLightAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (directionalLightPipeline != nullptr) {
			currentCommandBuffer->BeginDebugLabelSection("Directional Lighting", nullptr);
			currentCommandBuffer->BindGraphicsPipeline(directionalLightPipeline);

			std::array<GraphicsAPI::DescriptorSet*, 2> directionalLightDescriptors{};
			directionalLightDescriptors[0] = imageSet.gbufferDescriptorSet;

			auto view = registry.view<const entt::entity, const TransformComponent, DirectionalLightComponent>();
			view.each([&](const entt::entity entityHandle, const TransformComponent& transformComponent, DirectionalLightComponent& directionalLightComponent) {
				const ECS::Entity entity(entityHandle, scene);

				DirectionalLightComponent::UniformStruct lightStruct{
					bias * directionalLightComponent.shadowMatrix,
					directionalLightComponent.color,
					directionalLightComponent.sourceRadius,
					entity.GetWorldForward(),
					directionalLightComponent.intensity,
					static_cast<float>(directionalLightComponent.shadowResolution)
				};

				directionalLightComponent.uniformBufferObject->UploadData(&lightStruct);

				directionalLightDescriptors[1] = directionalLightComponent.descriptorSet;
				currentCommandBuffer->BindGraphicsDescriptorSet(directionalLightPipeline, directionalLightDescriptors.data(), 1, static_cast<uint32_t>(directionalLightDescriptors.size()));
				currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
				currentCommandBuffer->EndDebugLabelSection();
			});
		}
	}
}

void DeferredRenderer::RenderSsao(DeferredRendererImageSet& imageSet, GraphicsAPI::CommandBuffer* commandBuffer) {
	Grindstone::GraphicsPipelineAsset* ssaoPipelineSetAsset = ssaoPipelineSet.Get();
	Grindstone::GraphicsPipelineAsset* blurPipelineSetAsset = ssaoBlurPipelineSet.Get();
	if (ssaoPipelineSetAsset == nullptr || blurPipelineSetAsset == nullptr) {
		return;
	}

	Grindstone::GraphicsAPI::GraphicsPipeline* ssaoPipeline = ssaoPipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
	Grindstone::GraphicsAPI::GraphicsPipeline* blurPipeline = blurPipelineSetAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
	if (ssaoPipeline == nullptr || blurPipeline == nullptr) {
		return;
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	GraphicsAPI::ClearColor clearColorAttachment = { 16.0f, 16.0f, 16.0f, 16.0f };
	GraphicsAPI::ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;

	commandBuffer->BeginRendering(
		"SSAO Pass (Screen-Space Ambient Occlusion)",
		halfRenderArea,
		&imageSet.ambientOcclusionAttachment,
		1u
	);

	commandBuffer->SetViewport(halfRenderArea.offset.x, halfRenderArea.offset.y, static_cast<float>(halfRenderArea.GetWidth()), static_cast<float>(halfRenderArea.GetHeight()), 0.0f, 1.0f);
	commandBuffer->SetScissor(halfRenderArea.offset.x, halfRenderArea.offset.y, halfRenderArea.GetWidth(), halfRenderArea.GetHeight());

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> descriptorSets = {
		imageSet.engineDescriptorSet,
		imageSet.ssaoGbufferDescriptorSet,
		ssaoInputDescriptorSet
	};
	
	commandBuffer->BindGraphicsPipeline(ssaoPipeline);
	commandBuffer->BindGraphicsDescriptorSet(ssaoPipeline, descriptorSets.data(), 0, static_cast<uint32_t>(descriptorSets.size()));
	commandBuffer->DrawIndices(0, 6, 0, 1, 0);
	commandBuffer->EndRendering();

	commandBuffer->BeginRendering(
		"SSAO Blur Pass (Screen-Space Ambient Occlusion)",
		halfRenderArea,
		&imageSet.blurredAmbientOcclusionAttachment,
		1u,
		nullptr,
		nullptr
	);

	commandBuffer->SetViewport(halfRenderArea.offset.x, halfRenderArea.offset.y, static_cast<float>(halfRenderArea.GetWidth()), static_cast<float>(halfRenderArea.GetHeight()), 0.0f, 1.0f);
	commandBuffer->SetScissor(halfRenderArea.offset.x, halfRenderArea.offset.y, halfRenderArea.GetWidth(), halfRenderArea.GetHeight());

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	std::array<Grindstone::GraphicsAPI::DescriptorSet*, 2> blurDescriptorSets = {
		imageSet.engineDescriptorSet,
		imageSet.blurredSsaoInputDescriptorSet
	};

	commandBuffer->BindGraphicsPipeline(blurPipeline);
	commandBuffer->BindGraphicsDescriptorSet(blurPipeline, blurDescriptorSets.data(), 0, static_cast<uint32_t>(blurDescriptorSets.size()));
	commandBuffer->DrawIndices(0, 6, 0, 1, 0);
	commandBuffer->EndRendering();
}

void DeferredRenderer::RenderShadowMaps(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	AssetRendererManager* assetManager = engineCore.assetRendererManager;
	SceneManagement::Scene* scene = engineCore.GetSceneManager()->scenes.begin()->second;

	GraphicsAPI::ClearDepthStencil clearDepthStencil{};
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;

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

			pointLightComponent.shadowMapUniformBufferObject->UploadData(&shadowPass);

			commandBuffer->BeginRendering(
				"Point Shadow Pass",
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

			commandBuffer->BindGraphicsDescriptorSet(shadowMappingPipeline, &pointLightComponent.shadowMapDescriptorSet, 0, 1);
			assetManager->RenderShadowMap(
				commandBuffer,
				spotLightComponent.shadowMapDescriptorSet,
				registry,
				transformComponent.position
			);

			commandBuffer->EndRendering();
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

			spotLightComponent.shadowMatrix = projectionMatrix * viewMatrix;

			if (spotLightComponent.shadowResolution != spotLightComponent.cachedShadowResolution) {
				graphicsCore->WaitUntilIdle();
				spotLightComponent.shadowResolution = std::clamp(spotLightComponent.shadowResolution, 8u, 16192u);
				spotLightComponent.cachedShadowResolution = spotLightComponent.shadowResolution;
				spotLightComponent.depthTarget->Resize(spotLightComponent.shadowResolution, spotLightComponent.shadowResolution);
				spotLightComponent.framebuffer->Resize(spotLightComponent.shadowResolution, spotLightComponent.shadowResolution);
				GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::SampledImage(spotLightComponent.depthTarget);
				spotLightComponent.descriptorSet->ChangeBindings(&binding, 1, 1);
			}

			GraphicsAPI::ImageBarrier depthTargetBarrier = {
				.image = spotLightComponent.depthTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			};

			commandBuffer->PipelineBarrier(
				GraphicsAPI::PipelineStageBit::TopOfPipe,
				GraphicsAPI::PipelineStageBit::EarlyFragmentTests,
				nullptr, 0,
				&depthTargetBarrier, 1u
			);

			uint32_t resolution = spotLightComponent.shadowResolution;

			spotLightComponent.shadowMapUniformBufferObject->UploadData(&spotLightComponent.shadowMatrix);
			assetManager->SetEngineDescriptorSet(spotLightComponent.shadowMapDescriptorSet);

			Grindstone::GraphicsAPI::RenderAttachment shadowRenderAttachment{
				.image = spotLightComponent.depthTarget,
				.imageLayout =  Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = clearDepthStencil
			};

			Grindstone::Math::IntRect2D shadowRenderArea = Grindstone::Math::IntRect2D(0u, 0u, resolution, resolution);

			commandBuffer->BeginRendering(
				"Spot Light Pass",
				shadowRenderArea,
				nullptr,
				0u,
				&shadowRenderAttachment,
				nullptr
			);

			Grindstone::Rendering::RenderViewData renderViewData{
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.renderArea = shadowRenderArea
			};

			commandBuffer->SetViewport(0.0f, 0.0f, resolution, resolution);
			commandBuffer->SetScissor(0, 0, resolution, resolution);
			assetManager->RenderQueue(
				commandBuffer,
				renderViewData,
				registry,
				shadowMapRenderPassKey
			);

			commandBuffer->EndRendering();
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
			directionalLightComponent.shadowMatrix = projView;

			if (directionalLightComponent.shadowResolution != directionalLightComponent.cachedShadowResolution) {
				graphicsCore->WaitUntilIdle();
				directionalLightComponent.shadowResolution = std::clamp(directionalLightComponent.shadowResolution, 8u, 16192u);
				directionalLightComponent.cachedShadowResolution = directionalLightComponent.shadowResolution;
				directionalLightComponent.depthTarget->Resize(directionalLightComponent.shadowResolution, directionalLightComponent.shadowResolution);
				directionalLightComponent.framebuffer->Resize(directionalLightComponent.shadowResolution, directionalLightComponent.shadowResolution);
				GraphicsAPI::DescriptorSet::Binding binding = GraphicsAPI::DescriptorSet::Binding::SampledImage(directionalLightComponent.depthTarget);
				directionalLightComponent.descriptorSet->ChangeBindings(&binding, 1, 1);
			}

			GraphicsAPI::ImageBarrier depthTargetBarrier = {
				.image = directionalLightComponent.depthTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			};

			commandBuffer->PipelineBarrier(
				GraphicsAPI::PipelineStageBit::TopOfPipe,
				GraphicsAPI::PipelineStageBit::EarlyFragmentTests,
				nullptr, 0,
				&depthTargetBarrier, 1u
			);

			uint32_t resolution = directionalLightComponent.shadowResolution;

			directionalLightComponent.shadowMapUniformBufferObject->UploadData(&projView);
			assetManager->SetEngineDescriptorSet(directionalLightComponent.shadowMapDescriptorSet);

			Grindstone::GraphicsAPI::RenderAttachment shadowRenderAttachment{
				.image = directionalLightComponent.depthTarget,
				.imageLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
				.loadOp = Grindstone::GraphicsAPI::LoadOp::Clear,
				.storeOp = Grindstone::GraphicsAPI::StoreOp::Store,
				.clearValue = clearDepthStencil
			};

			float resF = static_cast<float>(resolution);
			Grindstone::Math::IntRect2D shadowRenderArea = Grindstone::Math::IntRect2D(0u, 0u, resolution, resolution);

			commandBuffer->BeginRendering(
				"Directional Shadow Map Pass",
				shadowRenderArea,
				nullptr,
				0u,
				&shadowRenderAttachment,
				nullptr
			);

			Grindstone::Rendering::RenderViewData renderViewData{
				.projectionMatrix = projectionMatrix,
				.viewMatrix = viewMatrix,
				.renderArea = shadowRenderArea,
			};

			commandBuffer->SetViewport(0.0f, 0.0f, resF, resF);
			commandBuffer->SetScissor(0, 0, resolution, resolution);
			assetManager->RenderQueue(
				commandBuffer,
				renderViewData,
				registry,
				shadowMapRenderPassKey
			);

			commandBuffer->EndRendering();
		});
	}
}

void DeferredRenderer::PostProcess(
	uint32_t imageIndex,
	GraphicsAPI::RenderAttachment& outRenderAttachment,
	GraphicsAPI::CommandBuffer* currentCommandBuffer
) {
	DeferredRendererImageSet& imageSet = deferredRendererImageSets[imageIndex];

	// RenderSsr(imageSet, commandBuffer);
	// RenderDepthOfField(imageSet, currentCommandBuffer);
	RenderBloom(imageSet, currentCommandBuffer);

	GraphicsAPI::ImageBarrier preTonemapImageBarrier{
		.image = outRenderAttachment.image,
		.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
		.newLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
		.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
		.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
		.imageAspect = GraphicsAPI::ImageAspectBits::Color,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	currentCommandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::TopOfPipe,
		GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
		nullptr, 0,
		&preTonemapImageBarrier, 1u
	);

	currentCommandBuffer->BeginRendering(
		"Tonemap Pass",
		renderArea,
		&outRenderAttachment,
		1u
	);

	Grindstone::GraphicsPipelineAsset* tonemapPipelineAsset = tonemapPipelineSet.Get();
	if (tonemapPipelineAsset != nullptr) {
		Grindstone::GraphicsAPI::GraphicsPipeline* tonemapPipeline = tonemapPipelineAsset->GetFirstPassPipeline(&vertexLightPositionLayout);
		if (tonemapPipeline != nullptr) {
			imageSet.tonemapPostProcessingUniformBufferObject->UploadData(&postProcessUboData);

			std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> descriptorSets{};
			descriptorSets[0] = imageSet.engineDescriptorSet;
			descriptorSets[1] = imageSet.gbufferDescriptorSet;
			descriptorSets[2] = imageSet.tonemapDescriptorSet;

			currentCommandBuffer->BindGraphicsPipeline(tonemapPipeline);
			currentCommandBuffer->BindGraphicsDescriptorSet(tonemapPipeline, descriptorSets.data(), 0, static_cast<uint32_t>(descriptorSets.size()));
			currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
		}
	}

	currentCommandBuffer->EndRendering();

	GraphicsAPI::ImageBarrier colorBarrier {
		.image = outRenderAttachment.image,
		.oldLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
		.newLayout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead,
		.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
		.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead,
		.imageAspect = GraphicsAPI::ImageAspectBits::Color,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	currentCommandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
		GraphicsAPI::PipelineStageBit::FragmentShader,
		nullptr, 0,
		&colorBarrier, 1u
	);

	GraphicsAPI::ImageBarrier depthBarrier{
		.image = imageSet.gbufferDepthStencilTarget,
		.oldLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead,
		.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
		.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead,
		.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
		.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	currentCommandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::FragmentShader,
		GraphicsAPI::PipelineStageBit::EarlyFragmentTests | GraphicsAPI::PipelineStageBit::LateFragmentTests,
		nullptr, 0,
		&depthBarrier, 1u
	);

	/*

	GraphicsAPI::ImageBarrier blitBarrier{
		.image = imageSet.gbufferDepthStencilTarget,
		.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	/*
	std::array<Grindstone::GraphicsAPI::ImageBarrier, 2> preBlitBarriers{ blitBarrier , blitBarrier };
	preBlitBarriers[0].image = imageSet.gbufferDepthStencilTarget;
	preBlitBarriers[0].oldLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead;
	preBlitBarriers[0].newLayout = Grindstone::GraphicsAPI::ImageLayout::TransferSrc;
	preBlitBarriers[0].srcAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite;
	preBlitBarriers[0].dstAccess = Grindstone::GraphicsAPI::AccessFlags::TransferRead;

	preBlitBarriers[1].image = framebuffer->GetDepthStencilTarget();
	preBlitBarriers[1].oldLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead;
	preBlitBarriers[1].newLayout = Grindstone::GraphicsAPI::ImageLayout::TransferDst;
	preBlitBarriers[1].srcAccess = Grindstone::GraphicsAPI::AccessFlags::None;
	preBlitBarriers[1].dstAccess = Grindstone::GraphicsAPI::AccessFlags::TransferWrite;

	std::array<Grindstone::GraphicsAPI::ImageBarrier, 2> postBlitBarriers{ blitBarrier , blitBarrier };
	postBlitBarriers[0].image = imageSet.gbufferDepthStencilTarget;
	postBlitBarriers[0].oldLayout = Grindstone::GraphicsAPI::ImageLayout::TransferSrc;
	postBlitBarriers[0].newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead;
	postBlitBarriers[0].srcAccess = Grindstone::GraphicsAPI::AccessFlags::TransferRead;
	postBlitBarriers[0].dstAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead;

	postBlitBarriers[1].image = framebuffer->GetDepthStencilTarget();
	postBlitBarriers[1].oldLayout = Grindstone::GraphicsAPI::ImageLayout::TransferDst;
	postBlitBarriers[1].newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead;
	postBlitBarriers[1].srcAccess = Grindstone::GraphicsAPI::AccessFlags::TransferWrite;
	postBlitBarriers[1].dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite;

	currentCommandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::LateFragmentTests,
		GraphicsAPI::PipelineStageBit::Transfer,
		nullptr, 0,
		preBlitBarriers.data(), static_cast<uint32_t>(preBlitBarriers.size())
	);
	currentCommandBuffer->BlitImage(
		imageSet.gbufferDepthStencilTarget,
		framebuffer->GetDepthStencilTarget(),
		Grindstone::GraphicsAPI::ImageLayout::TransferSrc,
		Grindstone::GraphicsAPI::ImageLayout::TransferDst,
		renderArea.GetWidth(), renderArea.GetHeight(), 1
	);
	currentCommandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::Transfer,
		GraphicsAPI::PipelineStageBit::FragmentShader | GraphicsAPI::PipelineStageBit::EarlyFragmentTests,
		nullptr, 0,
		postBlitBarriers.data(), static_cast<uint32_t>(postBlitBarriers.size())
	);
	*/
}

void DeferredRenderer::Debug(
	uint32_t imageIndex,
	GraphicsAPI::RenderAttachment& renderAttachment,
	GraphicsAPI::CommandBuffer* currentCommandBuffer
) {
	DeferredRendererImageSet& imageSet = deferredRendererImageSets[imageIndex];

	GraphicsAPI::ClearColor clearColor = { 0.3f, 0.6f, 0.9f, 1.f };
	GraphicsAPI::ClearDepthStencil clearDepthStencil;
	clearDepthStencil.depth = 1.0f;
	clearDepthStencil.stencil = 0;

	currentCommandBuffer->BeginRendering(
		"Debug Pass",
		renderArea,
		&renderAttachment,
		1u
	);

	Grindstone::GraphicsAPI::GraphicsPipeline* debugPipeline = debugPipelineSet.Get()->GetFirstPassPipeline(&vertexLightPositionLayout);
	if (debugPipeline != nullptr) {
		imageSet.debugUniformBufferObject->UploadData(&debugUboData);

		currentCommandBuffer->BindGraphicsDescriptorSet(debugPipeline, &imageSet.debugDescriptorSet, 2, 1);
		currentCommandBuffer->BindGraphicsPipeline(debugPipeline);
		currentCommandBuffer->DrawIndices(0, 6, 0, 1, 0);
	}

	currentCommandBuffer->EndRendering();
}

void DeferredRenderer::Render(
	GraphicsAPI::CommandBuffer* commandBuffer,
	entt::registry& registry,
	glm::mat4 projectionMatrix,
	glm::mat4 viewMatrix,
	glm::vec3 eyePos,
	GraphicsAPI::RenderAttachment& outRenderAttachment
) {
	Grindstone::EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::AssetRendererManager* assetManager = engineCore.assetRendererManager;
	GraphicsAPI::WindowGraphicsBinding* wgb = engineCore.windowManager->GetWindowByIndex(0)->GetWindowGraphicsBinding();

	graphicsCore->AdjustPerspective(&projectionMatrix[0][0]);

	uint32_t imageIndex = wgb->GetCurrentImageIndex();
	auto& imageSet = deferredRendererImageSets[imageIndex];

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

	imageSet.globalUniformBufferObject->UploadData(&engineUboStruct);

	Grindstone::Rendering::RenderViewData renderViewData{
		.projectionMatrix = projectionMatrix,
		.viewMatrix = viewMatrix,
		.renderArea = renderArea,
	};

	if (renderMode == DeferredRenderMode::Default) {
		RenderShadowMaps(commandBuffer, registry);
	}

	assetManager->SetEngineDescriptorSet(imageSet.engineDescriptorSet);

	{
		std::array<GraphicsAPI::ImageBarrier, 3> gbufferBarriers = {
			GraphicsAPI::ImageBarrier {
				.image = imageSet.gbufferAlbedoRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			GraphicsAPI::ImageBarrier {
				.image = imageSet.gbufferNormalRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			GraphicsAPI::ImageBarrier {
				.image = imageSet.gbufferSpecularRoughnessRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		commandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::TopOfPipe,
			GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
			nullptr, 0,
			gbufferBarriers.data(), static_cast<uint32_t>(gbufferBarriers.size())
		);

		GraphicsAPI::ImageBarrier depthBarrier{
			.image = imageSet.gbufferDepthStencilTarget,
			.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
			.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
			.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
			.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
			.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		commandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::TopOfPipe,
			GraphicsAPI::PipelineStageBit::EarlyFragmentTests,
			nullptr, 0,
			&depthBarrier, 1u
		);
	}

	{
		std::array<GraphicsAPI::RenderAttachment, 3> renderAttachments = {
			imageSet.gbufferAlbedoAttachment,
			imageSet.gbufferNormalAttachment,
			imageSet.gbufferSpecularRoughnessAttachment
		};

		commandBuffer->BeginRendering(
			"Gbuffer Geometry Pass",
			renderArea,
			renderAttachments.data(),
			static_cast<uint32_t>(renderAttachments.size()),
			&imageSet.gbufferDepthStencilAttachment
		);
	}

	commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderArea.GetWidth()), static_cast<float>(renderArea.GetHeight()));
	commandBuffer->SetScissor(0, 0, renderArea.GetWidth(), renderArea.GetHeight());

	imageSet.renderingStatsOpaque = assetManager->RenderQueue(commandBuffer, renderViewData, registry, geometryOpaqueRenderPassKey);
	commandBuffer->EndRendering();

	{
		std::array<GraphicsAPI::ImageBarrier, 3> gbufferBarriers = {
			GraphicsAPI::ImageBarrier {
				.image = imageSet.gbufferAlbedoRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			GraphicsAPI::ImageBarrier {
				.image = imageSet.gbufferNormalRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			GraphicsAPI::ImageBarrier {
				.image = imageSet.gbufferSpecularRoughnessRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		commandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
			GraphicsAPI::PipelineStageBit::FragmentShader,
			nullptr, 0,
			gbufferBarriers.data(), static_cast<uint32_t>(gbufferBarriers.size())
		);
	}

	std::vector<GraphicsAPI::ImageBarrier> depthBarriers;

	depthBarriers.emplace_back(GraphicsAPI::ImageBarrier{
		.image = imageSet.gbufferDepthStencilTarget,
		.oldLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
		.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead,
		.srcAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
		.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentRead,
		.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
		});

	{
		auto view = registry.view<const entt::entity, SpotLightComponent>();
		view.each([&](SpotLightComponent& spotLightComponent) {
			depthBarriers.emplace_back(
				GraphicsAPI::ImageBarrier{
					.image = spotLightComponent.depthTarget,
					.oldLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
					.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead,
					.srcAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
					.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentRead,
					.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			);
		});
	}

	{
		auto view = registry.view<DirectionalLightComponent>();
		view.each([&](DirectionalLightComponent& directionalLightComponent) {
			depthBarriers.emplace_back(
				GraphicsAPI::ImageBarrier{
					.image = directionalLightComponent.depthTarget,
					.oldLayout = Grindstone::GraphicsAPI::ImageLayout::DepthWrite,
					.newLayout = Grindstone::GraphicsAPI::ImageLayout::DepthRead,
					.srcAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentWrite,
					.dstAccess = Grindstone::GraphicsAPI::AccessFlags::DepthStencilAttachmentRead,
					.imageAspect = GraphicsAPI::ImageAspectBits::Depth,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			);
		});
	}

	commandBuffer->PipelineBarrier(
		GraphicsAPI::PipelineStageBit::LateFragmentTests,
		GraphicsAPI::PipelineStageBit::FragmentShader,
		nullptr, 0,
		depthBarriers.data(), static_cast<uint32_t>(depthBarriers.size())
	);

	if (renderMode == DeferredRenderMode::Default || renderMode == DeferredRenderMode::AmbientOcclusion) {
		RenderSsao(imageSet, commandBuffer);
	}

	commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderArea.GetWidth()), static_cast<float>(renderArea.GetHeight()));
	commandBuffer->SetScissor(0, 0, renderArea.GetWidth(), renderArea.GetHeight());

	{
		GraphicsAPI::ImageBarrier preLitHdrBufferBarrier{
				.image = imageSet.litHdrRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::Undefined,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::None,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
		};

		commandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::TopOfPipe,
			GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
			nullptr, 0,
			&preLitHdrBufferBarrier, 1u
		);

		GraphicsAPI::ClearColor clearColor{ 0.0f, 0.0f, 0.0f, 1.f };
		GraphicsAPI::ClearDepthStencil clearDepthStencil;

		commandBuffer->BeginRendering(
			"Lighting and Forward Pass",
			renderArea,
			&imageSet.litHdrAttachment,
			1u,
			&imageSet.forwardDepthStencilAttachment
		);

		if (renderMode == DeferredRenderMode::Default) {
			commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
			commandBuffer->BindIndexBuffer(indexBuffer);
			RenderLights(imageIndex, commandBuffer, registry);
		}

		imageSet.renderingStatsUnlit = assetManager->RenderQueue(commandBuffer, renderViewData, registry, geometryUnlitRenderPassKey);

		if (renderMode == DeferredRenderMode::Default) {
			imageSet.renderingStatsSky = assetManager->RenderQueue(commandBuffer, renderViewData, registry, geometrySkyRenderPassKey);
			imageSet.renderingStatsTransparent = assetManager->RenderQueue(commandBuffer, renderViewData, registry, geometryTransparentRenderPassKey);
		}

		commandBuffer->EndRendering();

		GraphicsAPI::ImageBarrier postLitHdrBufferBarrier{
				.image = imageSet.litHdrRenderTarget,
				.oldLayout = Grindstone::GraphicsAPI::ImageLayout::ColorAttachment,
				.newLayout = Grindstone::GraphicsAPI::ImageLayout::ShaderRead,
				.srcAccess = Grindstone::GraphicsAPI::AccessFlags::ColorAttachmentWrite,
				.dstAccess = Grindstone::GraphicsAPI::AccessFlags::ShaderRead,
				.imageAspect = GraphicsAPI::ImageAspectBits::Color,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
		};

		commandBuffer->PipelineBarrier(
			GraphicsAPI::PipelineStageBit::ColorAttachmentOutput,
			GraphicsAPI::PipelineStageBit::FragmentShader,
			nullptr, 0,
			&postLitHdrBufferBarrier, 1u
		);

	}

	commandBuffer->BindVertexBuffers(&vertexBuffer, 1);
	commandBuffer->BindIndexBuffer(indexBuffer);

	if (renderMode == DeferredRenderMode::Default) {
		PostProcess(imageIndex, outRenderAttachment, commandBuffer);
	}
	else {
		debugUboData.renderMode = static_cast<uint32_t>(renderMode);
		float projection_43 = projectionMatrix[3][2];
		float projection_33 = projectionMatrix[2][2];
		debugUboData.nearDistance = projection_43 / (projection_33 - 1.0f);
		debugUboData.farDistance = projection_43 / (projection_33 + 1.0f);
		Debug(imageIndex, outRenderAttachment, commandBuffer);
	}

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

	return {
		deferredRendererImageSets[imageIndex].renderingStatsOpaque,
		deferredRendererImageSets[imageIndex].renderingStatsUnlit,
		deferredRendererImageSets[imageIndex].renderingStatsTransparent,
		deferredRendererImageSets[imageIndex].renderingStatsSky
	};
}
