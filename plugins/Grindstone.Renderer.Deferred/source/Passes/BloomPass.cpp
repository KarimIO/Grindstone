#include <glm/glm.hpp>

#include <EngineCore/Assets/AssetManager.hpp>
#include <Grindstone.Renderer.Deferred/include/Passes/BloomPass.hpp>

const size_t MAX_BLOOM_MIPS = 16u;

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

bool Grindstone::Renderer::BloomPass::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	bloomPipelineSet = engineCore.assetManager->GetAssetReferenceByAddress<ComputePipelineAsset>("@CORESHADERS/postProcessing/bloom");

	return true;
}

/*
void Grindstone::Renderer::BloomPass::CreateDescriptorSets() {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();

	for (size_t i = 0; i < imageSet.bloomDescriptorSets.size(); ++i) {
		if (imageSet.bloomDescriptorSets[i] != nullptr) {
			graphicsCore->DeleteDescriptorSet(imageSet.bloomDescriptorSets[i]);
		}
	}

	imageSet.bloomDescriptorSets.resize(bloomStoredMipLevelCount * 2 - 2);
	std::array<GraphicsAPI::DescriptorSet::Binding, 5> descriptorBindings;
	descriptorBindings[1] = GraphicsAPI::DescriptorSet::Binding::Sampler(screenSampler);
	descriptorBindings[4] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[0]);
	/
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
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(bloomUniformBuffers[bloomDescriptorSetIndex]);
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage(imageSet.bloomRenderTargets[1]);
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.litHdrRenderTarget);
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}

	bloomUboStruct.stage = BloomStage::Downsample;
	for (size_t i = 1; i < bloomStoredMipLevelCount - 1; ++i) {
		std::string bloomDescriptorName = std::vformat("Bloom DS Downsample [{}]({})", std::make_format_args(imageSetIndex, i));
		descriptorSetCreateInfo.debugName = bloomDescriptorName.c_str();
		bloomUniformBuffers[bloomDescriptorSetIndex]->UploadData(&bloomUboStruct);
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(bloomUniformBuffers[bloomDescriptorSetIndex]);
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage(imageSet.bloomRenderTargets[i + 1]);
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[i]);
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
		descriptorBindings[0] = GraphicsAPI::DescriptorSet::Binding::UniformBuffer(bloomUniformBuffers[bloomDescriptorSetIndex]);
		descriptorBindings[2] = GraphicsAPI::DescriptorSet::Binding::StorageImage(imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i]);
		descriptorBindings[3] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[bloomStoredMipLevelCount + i + 1]);
		descriptorBindings[4] = GraphicsAPI::DescriptorSet::Binding::SampledImage(imageSet.bloomRenderTargets[i]);
		imageSet.bloomDescriptorSets[bloomDescriptorSetIndex++] = graphicsCore->CreateDescriptorSet(descriptorSetCreateInfo);
	}
}

void Grindstone::Renderer::BloomPass::UpdateBloomUBO() {
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

void Grindstone::Renderer::BloomPass::CreateBloomResources() {
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

void Grindstone::Renderer::BloomPass::UpdateBloomDescriptorSet() {
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

void Grindstone::Renderer::BloomPass::CreateUniformBuffers() {
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

void DrawGbufferPass() {
	struct GbufferData {
		Renderer::RenderGraphResource albedo;
		Renderer::RenderGraphResource normal;
		Renderer::RenderGraphResource specularRoughness;
		Renderer::RenderGraphResource depth;
	};

	Grindstone::Renderer::RenderGraphPass& gbufferPass = renderGraph.AddCallbackPass<GbufferData>(
		"Gbuffer Geometry Pass",
		[&](Renderer::RenderGraph::Builder& builder, GbufferData& data) {
			gbufferPass.AddOutputImage(attachmentNameAlbedo, attachmentAlbedo);
			gbufferPass.AddOutputImage(attachmentNameNormal, attachmentNormal);
			gbufferPass.AddOutputImage(attachmentNameSpecularRoughness, attachmentSpecularRoughness);
			gbufferPass.AddOutputImage(attachmentNameDepthStencil, attachmentDepthStencil);
		},
		[&, registry = &registry](const GbufferData& data, Renderer::RenderGraphResource& resources) {
			std::array<GraphicsAPI::RenderAttachment, 3> renderAttachments = {
				resources.Get<RenderGraphImage>(data.albedo).image,
				resources.Get<RenderGraphImage>(data.normal).image,
				resources.Get<RenderGraphImage>(data.specularRoughness).image
			};

			commandBuffer->BeginRendering(
				"Gbuffer Geometry Pass",
				renderArea,
				renderAttachments.data(),
				static_cast<uint32_t>(renderAttachments.size()),
				&resources.Get<RenderGraphImage>(data.depth).image
			);

			commandBuffer->SetViewport(0.0f, 0.0f, static_cast<float>(renderArea.GetWidth()), static_cast<float>(renderArea.GetHeight()));
			commandBuffer->SetScissor(0, 0, renderArea.GetWidth(), renderArea.GetHeight());

			// TODO: Get Rendering Stats
			assetManager->RenderQueue(commandBuffer, renderViewData, *registry, geometryOpaqueRenderPassKey);
			commandBuffer->EndRendering();
		}
	);
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
*/
