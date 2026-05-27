#include <glm/glm.hpp>

#include <EngineCore/Assets/AssetManager.hpp>
#include <Common/Graphics/Core.hpp>

#include <Grindstone.Renderer.Deferred/include/Passes/BloomPass.hpp>

using namespace Grindstone::Renderer;

enum class BloomStage : uint32_t {
	Filter = 0,
	Downsample,
	Upsample
};

// Data used across stages - shape of bloom.
struct BloomDataUboStruct {
	glm::vec4 thresholdFilter; // (x) threshold, (y) threshold - knee, (z) knee * 2, (w) 0.25 / knee
	float levelOfDetail;
	float filterRadius;
};

// Data used per-stage - input image sizes and stage type.
struct BloomStageUboStruct {
	glm::vec2 inReciprocalImgSize;
	glm::vec2 outReciprocalImgSize;
	BloomStage stage;
};

bool Grindstone::Renderer::BloomPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	bloomPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/bloom");

	Grindstone::GraphicsAPI::Sampler::CreateInfo screenSamplerCreateInfo{
	screenSamplerCreateInfo.debugName = "Bloom Screen Sampler",
	screenSamplerCreateInfo.options = {
			.wrapModeU = GraphicsAPI::TextureWrapMode::ClampToEdge,
			.wrapModeV = GraphicsAPI::TextureWrapMode::ClampToEdge,
			.wrapModeW = GraphicsAPI::TextureWrapMode::ClampToEdge,
			.minFilter = GraphicsAPI::TextureFilter::Linear,
			.magFilter = GraphicsAPI::TextureFilter::Linear,
			.anistropy = 0
		}
	};
	screenSampler = engineCore.GetGraphicsCore()->GetOrCreateSampler(screenSamplerCreateInfo);

	// Threshold values sourced from: https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/
	float threshold = 1.0f;
	float softThreshold = 0.5f;
	float knee = threshold * softThreshold;
	float thresholdBias = 0.00001f;

	BloomDataUboStruct bloomUboStruct{
		.thresholdFilter = { threshold, threshold - knee, 2.0f * knee, 0.25f / (knee + thresholdBias) },
		.levelOfDetail = 0.0f,
		.filterRadius = 0.005f
	};

	GraphicsAPI::Buffer::CreateInfo bloomDataBufferCreateInfo{
		.content = &bloomUboStruct,
		.bufferSize = sizeof(BloomDataUboStruct),
		.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform,
		.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU
	};

	GraphicsAPI::Buffer::CreateInfo bloomStageBufferCreateInfo{
		.bufferSize = sizeof(BloomStageUboStruct),
		.bufferUsage =
			GraphicsAPI::BufferUsage::TransferDst |
			GraphicsAPI::BufferUsage::TransferSrc |
			GraphicsAPI::BufferUsage::Uniform,
		.memoryUsage = GraphicsAPI::MemoryUsage::CPUToGPU
	};

	std::array<GraphicsAPI::DescriptorSetLayout::Binding, 2> descriptorLayoutBindings{
		GraphicsAPI::DescriptorSetLayout::Binding{
			.bindingId = 0u,
			.count = 1u,
			.type = GraphicsAPI::BindingType::UniformBuffer,
			.stages = GraphicsAPI::ShaderStageBit::Compute
		},
		GraphicsAPI::DescriptorSetLayout::Binding{
			.bindingId = 1u,
			.count = 1u,
			.type = GraphicsAPI::BindingType::UniformBuffer,
			.stages = GraphicsAPI::ShaderStageBit::Compute
		}
	};
	GraphicsAPI::DescriptorSetLayout::CreateInfo bloomDescriptorSetLayoutCreateInfo{
		.debugName = "Bloom Data Descriptor Set Layout",
		.bindings = descriptorLayoutBindings.data(),
		.bindingCount = static_cast<uint32_t>(descriptorLayoutBindings.size())
	};
	descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(bloomDescriptorSetLayoutCreateInfo);

	std::array<GraphicsAPI::DescriptorSet::Binding, 2> descriptorBindings{
		GraphicsAPI::DescriptorSet::Binding(nullptr, GraphicsAPI::BindingType::UniformBuffer, 1u),
		GraphicsAPI::DescriptorSet::Binding(nullptr, GraphicsAPI::BindingType::UniformBuffer, 1u)
	};

	GraphicsAPI::DescriptorSet::CreateInfo bloomDescriptorSetCreateInfo{
		.layout = descriptorSetLayout,
		.bindings = descriptorBindings.data(),
		.bindingCount = static_cast<uint32_t>(descriptorBindings.size())
	};

	for (size_t imageSetIndex = 0; imageSetIndex < imageSets.size(); ++imageSetIndex) {
		ImageSet& imageSet = imageSets[imageSetIndex];

		std::string dataBufferName = std::format("Bloom Data Uniform Buffer [{}]", imageSetIndex);
		bloomDataBufferCreateInfo.debugName = dataBufferName.c_str();
		imageSet.bloomDataBuffer = graphicsCore->CreateBuffer(bloomDataBufferCreateInfo);
		descriptorBindings[0].itemPtr = imageSet.bloomDataBuffer;

		for (size_t stageIndex = 0; stageIndex < BLOOM_STAGE_COUNT; ++stageIndex) {
			GraphicsAPI::Buffer*& stageBuffer = imageSet.bloomStageBuffers[stageIndex];
			GraphicsAPI::DescriptorSet*& descriptorSet = imageSet.descriptorSets[stageIndex];

			std::string stageBufferName = std::format("Bloom Stage Uniform Buffer {} [{}]", stageIndex, imageSetIndex);
			bloomStageBufferCreateInfo.debugName = stageBufferName.c_str();
			stageBuffer = graphicsCore->CreateBuffer(bloomStageBufferCreateInfo);
			descriptorBindings[1].itemPtr = stageBuffer;

			std::string descriptorSetName = std::format("Bloom Descriptor Set Layout {} [{}]", stageIndex, imageSetIndex);
			bloomDescriptorSetCreateInfo.debugName = descriptorSetName.c_str();
			descriptorSet = graphicsCore->CreateDescriptorSet(bloomDescriptorSetCreateInfo);
		}
	}

	return true;
}

Grindstone::Renderer::ImageDescription genericBloomImageDescription{
	.format = Grindstone::GraphicsAPI::Format::R32G32B32A32_SFLOAT,
	.imageUsage =
		Grindstone::GraphicsAPI::ImageUsageFlags::RenderTarget |
		Grindstone::GraphicsAPI::ImageUsageFlags::Sampled |
		Grindstone::GraphicsAPI::ImageUsageFlags::Storage
};

static RenderGraphBuilderResourceRef BloomOperation(
	const Grindstone::String& passName,
	Grindstone::Math::Uint2 size,
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet,
	Grindstone::GraphicsAPI::Sampler* sampler,
	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline,
	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input1,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input2
) {
	return renderGraphBuilder.CreateComputePass<RenderGraphBuilderResourceRef>(
		passName,
		[input1, input2, sampler, size, &passName](ComputeRenderGraphBuilderPass<RenderGraphBuilderResourceRef>& pass) -> RenderGraphBuilderResourceRef {
			ImageDescription resourceDesc = genericBloomImageDescription;
			resourceDesc.name = passName;
			resourceDesc.size = MetaSize2D::Pixels(size.x, size.y);
			pass.ReadExternalSampler(sampler);
			auto outputRef = pass.WriteStorageImage(resourceDesc);
			pass.ReadSampledImage(input1);
			pass.ReadSampledImage(input2);

			return outputRef;
		},
		[size, descriptorSet, bloomPipeline, pipelineLayout](
			RenderGraphContext& cxt,
			const RenderGraphFrameResources& frameResources,
			RenderGraphBuilderResourceRef& ref
		) {
			Grindstone::GraphicsAPI::CommandBuffer* cmd = cxt.commandBuffer;

			constexpr uint32_t WORKGROUP_SIZE = 4;
			uint32_t groupCountX = (size.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
			uint32_t groupCountY = (size.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

			cmd->BindComputePipeline(bloomPipeline);
			cmd->BindComputeDescriptorSet(pipelineLayout, &descriptorSet, 2u, 1u);
			cmd->DispatchCompute(groupCountX, groupCountY, 1u);
		}
	);
}

static RenderGraphBuilderResourceRef FirstDownscale(
	Grindstone::Math::Uint2 size,
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet,
	Grindstone::GraphicsAPI::Sampler* sampler,
	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline,
	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input
) {
	Grindstone::String passName = "Bloom Downscale First Pass";
	return BloomOperation(passName, size, descriptorSet, sampler, bloomPipeline, pipelineLayout, renderGraphBuilder, input, input);
}

static RenderGraphBuilderResourceRef Downscale(
	uint32_t downscaleIndex,
	Grindstone::Math::Uint2 size,
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet,
	Grindstone::GraphicsAPI::Sampler* sampler,
	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline,
	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input
) {
	Grindstone::String passName = std::format("Bloom Downscale {} Pass", downscaleIndex);
	return BloomOperation(passName, size, descriptorSet, sampler, bloomPipeline, pipelineLayout, renderGraphBuilder, input, input);
}

static RenderGraphBuilderResourceRef FirstUpscale(
	Grindstone::Math::Uint2 size,
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet,
	Grindstone::GraphicsAPI::Sampler* sampler,
	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline,
	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input1,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input2
) {
	Grindstone::String passName = "Bloom Upscale First Pass";
	return BloomOperation(passName, size, descriptorSet, sampler, bloomPipeline, pipelineLayout, renderGraphBuilder, input1, input2);
}

static RenderGraphBuilderResourceRef Upscale(
	uint32_t upscaleIndex,
	Grindstone::Math::Uint2 size,
	Grindstone::GraphicsAPI::DescriptorSet* descriptorSet,
	Grindstone::GraphicsAPI::Sampler* sampler,
	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline,
	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input1,
	Grindstone::Renderer::RenderGraphBuilderResourceRef input2
) {
	Grindstone::String passName = std::format("Bloom Upscale {} Pass", upscaleIndex);
	return BloomOperation(passName, size, descriptorSet, sampler, bloomPipeline, pipelineLayout, renderGraphBuilder, input1, input2);
}

static void ResizeUbo(BloomStage stage, Grindstone::Math::Uint2 inSize, Grindstone::Math::Uint2 outSize, Grindstone::GraphicsAPI::Buffer* buffer) {
	BloomStageUboStruct stageData{
		.inReciprocalImgSize = glm::vec2(1.0f / inSize.x, 1.0f / inSize.y),
		.outReciprocalImgSize = glm::vec2(1.0f / outSize.x, 1.0f / outSize.y),
		.stage = stage
	};
	buffer->UploadData(&stageData);
}

static void InitializeImageSetIfNecessary(
	Grindstone::Math::Uint2 size,
	std::array<Grindstone::Math::Uint2, Grindstone::Renderer::BloomPass::BLOOM_MIPS + 1>& mipSizes,
	Grindstone::Renderer::BloomPass::ImageSet& set
) {
	if (size == set.size) {
		return;
	}

	auto n = Grindstone::Renderer::BloomPass::BLOOM_MIPS;

	auto& buff = set.bloomStageBuffers;
	// TODO: Fix mipSizes
	ResizeUbo(BloomStage::Filter, mipSizes[0], mipSizes[1], buff[0]);
	for (size_t i = 1; i < n; ++i) {
		ResizeUbo(BloomStage::Downsample, mipSizes[i + 1], mipSizes[i + 1], buff[i]);
	}
	for (size_t i = 0; i < n - 1u; ++i) {
		ResizeUbo(BloomStage::Upsample, mipSizes[n - i], mipSizes[n - i - 1], buff[4 + i]);
	}
}

Grindstone::Renderer::RenderGraphBuilderResourceRef Grindstone::Renderer::BloomPass::AddBloomChain(
	uint32_t imageIndex,
	Grindstone::Math::Uint2 size,
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Renderer::RenderGraphBuilderResourceRef input
) {
	Grindstone::ComputePipelineAsset* bloomPipelineAsset = bloomPipelineSet.Get();
	if (bloomPipelineAsset == nullptr) {
		return input;
	}

	Grindstone::GraphicsAPI::ComputePipeline* bloomPipeline = bloomPipelineAsset->GetPipeline();
	if (bloomPipeline == nullptr) {
		return input;
	}

	Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = bloomPipelineAsset->GetPipelineLayout();

	std::array<Grindstone::Renderer::RenderGraphBuilderResourceRef, BLOOM_STAGE_COUNT> stageOutputs{};
	std::array<Grindstone::Math::Uint2, BLOOM_MIPS + 1> mipSizes{
		Grindstone::Math::Uint2{size.x, size.y},
		Grindstone::Math::Uint2{size.x >> 1, size.y >> 1},
		Grindstone::Math::Uint2{size.x >> 2, size.y >> 2},
		Grindstone::Math::Uint2{size.x >> 3, size.y >> 3},
		Grindstone::Math::Uint2{size.x >> 4, size.y >> 4}
	};

	// Don't process if the image is too small.
	auto& lastMipSize = mipSizes[BLOOM_MIPS - 1];
	if (lastMipSize.x == 0 || lastMipSize.y == 0) {
		return input;
	}

	auto& imageSet = imageSets[imageIndex];
	InitializeImageSetIfNecessary(size, mipSizes, imageSet);

	Grindstone::GraphicsAPI::DescriptorSet** ds = imageSet.descriptorSets.data();
	
	Renderer::RenderGraphBuilderResourceRef outRef;
	stageOutputs[0] = outRef = FirstDownscale(mipSizes[1], *(ds++), screenSampler, bloomPipeline, pipelineLayout, renderGraphBuilder, input);

	for (uint32_t i = 1; i < BLOOM_MIPS; ++i) {
		stageOutputs[i] = outRef = Downscale(i, mipSizes[i + 1u], *(ds++), screenSampler, bloomPipeline, pipelineLayout, renderGraphBuilder, outRef);
	}

	stageOutputs[BLOOM_MIPS] = outRef = FirstUpscale(mipSizes[BLOOM_MIPS - 1u], *(ds++), screenSampler, bloomPipeline, pipelineLayout, renderGraphBuilder, outRef, stageOutputs[BLOOM_MIPS - 2u]);

	for (uint32_t i = 1; i < BLOOM_MIPS - 1u; ++i) {
		stageOutputs[BLOOM_MIPS + i] = outRef = Upscale(i, mipSizes[BLOOM_MIPS - i - 1u], *(ds++), screenSampler, bloomPipeline, pipelineLayout, renderGraphBuilder, outRef, stageOutputs[BLOOM_MIPS - 1u - i]);
	}

	return outRef;
}
