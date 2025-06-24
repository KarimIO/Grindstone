#include <filesystem>

#include <Common/Containers/Span.hpp>
#include <Common/Formats/PipelineSet.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>

#include "GraphicsPipelineImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Containers;
using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Formats::Pipelines;

static void UnpackGraphicsPipelineHeader(
	const V1::PassPipelineHeader& srcHeader,
	GraphicsPipeline::PipelineData& dstPipelineData
) {
	dstPipelineData.primitiveType = srcHeader.geometryType;
	dstPipelineData.polygonFillMode = srcHeader.polygonFillMode;
	dstPipelineData.depthCompareOp = srcHeader.depthCompareOp;
	dstPipelineData.cullMode = srcHeader.cullMode;
	dstPipelineData.depthBiasClamp = srcHeader.depthBiasClamp;
	dstPipelineData.depthBiasConstantFactor = srcHeader.depthBiasConstantFactor;
	dstPipelineData.depthBiasSlopeFactor = srcHeader.depthBiasSlopeFactor;

	dstPipelineData.isDepthBiasEnabled = srcHeader.flags & 0b1;
	dstPipelineData.isDepthClampEnabled = srcHeader.flags & 0b10;
	dstPipelineData.isDepthTestEnabled = srcHeader.flags & 0b100;
	dstPipelineData.isDepthWriteEnabled = srcHeader.flags & 0b1000;
	dstPipelineData.isStencilEnabled = srcHeader.flags & 0b10000;

	// TODO: We should check this and probably apply the first attachment data to others if necessary.
	// dstHeader.shouldCopyFirstAttachment ? 0b100000;
}

static void UnpackGraphicsPipelineAttachmentHeaders(
	const Span<V1::PassPipelineAttachmentHeader>& srcAttachments,
	Grindstone::GraphicsAPI::GraphicsPipeline::AttachmentData* dstAttachments
) {
	for (uint8_t i = 0; i < srcAttachments.GetSize(); ++i) {
		const V1::PassPipelineAttachmentHeader& srcHeader = srcAttachments[i];
		Grindstone::GraphicsAPI::GraphicsPipeline::AttachmentData& dstAttachment = dstAttachments[i];

		dstAttachment.colorMask = srcHeader.colorMask;
		dstAttachment.blendData.alphaFactorDst = srcHeader.blendAlphaFactorDst;
		dstAttachment.blendData.alphaFactorSrc = srcHeader.blendAlphaFactorSrc;
		dstAttachment.blendData.alphaOperation = srcHeader.blendAlphaOperation;
		dstAttachment.blendData.colorFactorDst = srcHeader.blendColorFactorDst;
		dstAttachment.blendData.colorFactorSrc = srcHeader.blendColorFactorSrc;
		dstAttachment.blendData.colorOperation = srcHeader.blendColorOperation;
	}
}

static void UnpackGraphicsPipelineDescriptorSetHeaders(
	Grindstone::GraphicsAPI::Core* graphicsCore,
	std::string_view pipelineName,
	const Span<V1::ShaderReflectDescriptorSet>& srcDescriptorSets,
	const Span<V1::ShaderReflectDescriptorBinding>& srcDescriptorBindings,
	std::array<Grindstone::GraphicsAPI::DescriptorSetLayout*, 16>& dstDescriptorSets
) {
	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCreateInfo;

	for (uint8_t i = 0; i < srcDescriptorSets.GetSize(); ++i) {
		const V1::ShaderReflectDescriptorSet& srcDescriptorSet = srcDescriptorSets[i];

		std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> dstDescriptorBindings;
		dstDescriptorBindings.reserve(srcDescriptorSet.bindingCount);

		for (size_t bindingIndex = srcDescriptorSet.bindingStartIndex;
			bindingIndex < srcDescriptorSet.bindingStartIndex + srcDescriptorSet.bindingCount;
			++bindingIndex
		) {
			const V1::ShaderReflectDescriptorBinding& srcBinding = srcDescriptorBindings[bindingIndex];
			Grindstone::GraphicsAPI::DescriptorSetLayout::Binding& dstBinding = dstDescriptorBindings.emplace_back();
			dstBinding.bindingId = srcBinding.bindingIndex;
			dstBinding.stages = srcBinding.stages;
			dstBinding.count = srcBinding.count;
			dstBinding.type = srcBinding.type;
		}

		std::string setName = fmt::format("{} Descriptor Set {}", pipelineName, i);
		layoutCreateInfo.debugName = setName.c_str();
		layoutCreateInfo.bindings = dstDescriptorBindings.data();
		layoutCreateInfo.bindingCount = static_cast<uint32_t>(dstDescriptorBindings.size());

		dstDescriptorSets[srcDescriptorSet.setIndex] = graphicsCore->CreateDescriptorSetLayout(layoutCreateInfo);
	}
}

// Graphics Pipeline Format:
// GPSF character magic text
// GraphicsPipelineFileHeader
// GraphicsPipelineShaderStageHeaders[]
// GraphicsPipelineAttachmentFileHeader[]
// Shader Code[]

static bool ImportGraphicsPipelineAsset(GraphicsPipelineAsset& graphicsPipelineAsset) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore.assetManager;
	AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;
	RenderPassRegistry* renderPassRegistry = Grindstone::EngineCore::GetInstance().GetRenderPassRegistry();

	Assets::AssetLoadBinaryResult result = assetManager->LoadBinaryByUuid(AssetType::GraphicsPipelineSet, graphicsPipelineAsset.uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find shader with id {}.", graphicsPipelineAsset.uuid.ToString());
		graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	Grindstone::Buffer& fileData = result.buffer;
	GS_ASSERT(fileData.GetCapacity() >= (4 + sizeof(V1::PipelineSetFileHeader)));

	if (memcmp(fileData.Get(), V1::FileMagicCode, 4) != 0) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Graphics Pipeline file does not start with GPSF - {}.", result.displayName);
		graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	graphicsPipelineAsset.name = result.displayName;

	V1::PipelineSetFileHeader* srcFileHeader = fileData.Get<V1::PipelineSetFileHeader>(4);
	
	GS_ASSERT(srcFileHeader->headerSize == sizeof(V1::PipelineSetFileHeader));
	GS_ASSERT(srcFileHeader->graphicsPipelineSize == sizeof(V1::GraphicsPipelineSetHeader));
	GS_ASSERT(srcFileHeader->computePipelineSize == sizeof(V1::ComputePipelineSetHeader));
	GS_ASSERT(srcFileHeader->graphicsConfigurationSize == sizeof(V1::GraphicsPipelineConfigurationHeader));
	GS_ASSERT(srcFileHeader->computeConfigurationSize == sizeof(V1::ComputePipelineConfigurationHeader));
	GS_ASSERT(srcFileHeader->passSize == sizeof(V1::PassPipelineHeader));
	GS_ASSERT(srcFileHeader->attachmentSize == sizeof(V1::PassPipelineAttachmentHeader));
	GS_ASSERT(srcFileHeader->stageSize == sizeof(V1::PassPipelineShaderStageHeader));

	Span<V1::GraphicsPipelineSetHeader> graphicsPipelines = fileData.GetSpan<V1::GraphicsPipelineSetHeader>(srcFileHeader->graphicsPipelinesOffset, srcFileHeader->graphicsPipelineCount);
	Span<V1::GraphicsPipelineConfigurationHeader> pipelineConfigurations = fileData.GetSpan<V1::GraphicsPipelineConfigurationHeader>(srcFileHeader->graphicsConfigurationsOffset, srcFileHeader->graphicsConfigurationCount);
	Span<V1::PassPipelineHeader> pipelinePasses = fileData.GetSpan<V1::PassPipelineHeader>(srcFileHeader->graphicsPassesOffset, srcFileHeader->graphicsPassCount);
	Span<V1::PassPipelineShaderStageHeader> shaderStages = fileData.GetSpan<V1::PassPipelineShaderStageHeader>(srcFileHeader->shaderStagesOffset, srcFileHeader->shaderStageCount);
	Span<V1::PassPipelineAttachmentHeader> attachments = fileData.GetSpan<V1::PassPipelineAttachmentHeader>(srcFileHeader->attachmentHeadersOffset, srcFileHeader->attachmentHeaderCount);
	Span<V1::ShaderReflectDescriptorSet> descriptorSets = fileData.GetSpan<V1::ShaderReflectDescriptorSet>(srcFileHeader->descriptorSetsOffset, srcFileHeader->descriptorSetCount);
	Span<V1::ShaderReflectDescriptorBinding> descriptorBindings = fileData.GetSpan<V1::ShaderReflectDescriptorBinding>(srcFileHeader->descriptorBindingsOffset, srcFileHeader->descriptorBindingCount);
	Span<V1::MaterialParameter> materialParameters = fileData.GetSpan<V1::MaterialParameter>(srcFileHeader->materialParametersOffset, srcFileHeader->materialParameterCount);
	Span<V1::MaterialResource> materialResources = fileData.GetSpan<V1::MaterialResource>(srcFileHeader->materialResourcesOffset, srcFileHeader->materialResourceCount);
	BufferSpan blobs = fileData.GetSpan(srcFileHeader->blobSectionOffset, srcFileHeader->blobSectionSize);

	GS_ASSERT(graphicsPipelines.GetSize() != 0);
	const V1::GraphicsPipelineSetHeader& srcPipelineHeader = graphicsPipelines[0];

	GS_ASSERT(srcPipelineHeader.configurationCount != 0);
	const V1::GraphicsPipelineConfigurationHeader& srcConfigHeader = pipelineConfigurations[0];

	// TODO: Encapsulate this in a function.
	// Prepare Meta data
	{
		graphicsPipelineAsset.metaData.materialBufferIndex = GraphicsPipelineAsset::noMaterialBufferIndex;

		if (materialParameters.GetSize() > 0) {
			graphicsPipelineAsset.metaData.materialBufferIndex = 1; // TODO: How to get?
		}

		graphicsPipelineAsset.metaData.buffers.resize(1); // TODO: Number of buffers
		Grindstone::PipelineAssetMetaData::Buffer& bufferMetadata = graphicsPipelineAsset.metaData.buffers.emplace_back();

		for (V1::MaterialResource& srcMaterialResource : materialResources) {
			auto& dstResource = graphicsPipelineAsset.metaData.resources.emplace_back();
			dstResource.slotName = reinterpret_cast<const char*>(&blobs[srcMaterialResource.nameOffsetFromBlobStart]);
			dstResource.descriptorSet = srcMaterialResource.setIndex;
			dstResource.descriptorBinding = srcMaterialResource.bindingIndex;
		}
	}

	// TODO: Loop over Configurations
	graphicsPipelineAsset.passes.resize(srcConfigHeader.passCount);
	for (uint32_t passIndex = srcConfigHeader.passStartIndex; passIndex < srcConfigHeader.passCount; ++passIndex) {
		GraphicsPipelineAsset::Pass& pass = graphicsPipelineAsset.passes[passIndex];
		const V1::PassPipelineHeader& srcPass = pipelinePasses[passIndex];
		const char* passName = reinterpret_cast<const char*>(&blobs[srcPass.pipelineNameOffsetFromBlobStart]);
		const char* renderQueueName = reinterpret_cast<const char*>(&blobs[srcPass.renderQueueNameOffsetFromBlobStart]);
		pass.passPipelineName = result.displayName + " " + passName;
		pass.renderQueue = renderQueueName;

		GraphicsPipeline::PipelineData& pipelineData = pass.pipelineData;
		pipelineData.debugName = result.displayName.c_str();
		pipelineData.width = 0.0f;
		pipelineData.height = 0.0f;
		pipelineData.scissorX = 0;
		pipelineData.scissorY = 0;
		pipelineData.scissorW = 0;
		pipelineData.scissorH = 0;
		pipelineData.hasDynamicViewport = true;
		pipelineData.hasDynamicScissor = true;
		pipelineData.renderPass = renderPassRegistry->GetRenderpass(renderQueueName);

		// Get the highest descriptor set index
		pipelineData.descriptorSetLayoutCount = 0;
		for (uint8_t descriptorSetIndex = 0; descriptorSetIndex < srcPass.descriptorSetCount; ++descriptorSetIndex) {
			auto& descriptorSet = descriptorSets[descriptorSetIndex];
			uint32_t indexAsCount = descriptorSet.setIndex + 1;
			pipelineData.descriptorSetLayoutCount = pipelineData.descriptorSetLayoutCount > indexAsCount
				? pipelineData.descriptorSetLayoutCount
				: indexAsCount;
		}

		for (uint8_t descriptorSetIndex = 0; descriptorSetIndex < pipelineData.descriptorSetLayoutCount; ++descriptorSetIndex) {
			GraphicsAPI::DescriptorSetLayout*& descriptorSetLayout = pass.descriptorSetLayouts[descriptorSetIndex];
			if (descriptorSetLayout == nullptr) {
				GraphicsAPI::DescriptorSetLayout::CreateInfo ci{};
				ci.debugName = "Unbound descriptor set";
				ci.bindingCount = 0;
				ci.bindings = nullptr;
				descriptorSetLayout = graphicsCore->CreateDescriptorSetLayout(ci);
			}
		}

		pipelineData.shaderStageCreateInfos = nullptr;
		pipelineData.shaderStageCreateInfoCount = srcPass.shaderStageCount;
		pipelineData.colorAttachmentCount = srcPass.attachmentCount;

		UnpackGraphicsPipelineHeader(srcPass, pipelineData);

		for (uint8_t i = 0; i < srcPass.shaderStageCount; ++i) {
			V1::PassPipelineShaderStageHeader& srcStage = shaderStages[srcPass.shaderStageStartIndex + i];
			pass.stageBuffers[i] = Grindstone::Buffer(
				reinterpret_cast<void*>(&blobs[srcStage.shaderCodeOffsetFromBlobStart]),
				static_cast<const uint64_t>(srcStage.shaderCodeSize)
			);
			pass.stageTypes[i] = srcStage.stageType;
		}


		auto& colorAttachmentData = pass.colorAttachmentData;
		GS_ASSERT(srcPass.attachmentCount <= pass.colorAttachmentData.size());
		UnpackGraphicsPipelineAttachmentHeaders(
			attachments.GetSubspan(srcPass.attachmentStartIndex, srcPass.attachmentCount),
			colorAttachmentData.data()
		);

		auto& descriptorSetLayouts = pass.descriptorSetLayouts;
		GS_ASSERT(srcPass.descriptorSetCount <= pass.descriptorSetLayouts.size());
		UnpackGraphicsPipelineDescriptorSetHeaders(
			graphicsCore,
			graphicsPipelineAsset.name,
			descriptorSets.GetSubspan(srcPass.descriptorSetStartIndex, srcPass.descriptorSetCount),
			descriptorBindings,
			descriptorSetLayouts
		);

		pass.passPipelineName = result.displayName;
		pipelineData.colorAttachmentData = colorAttachmentData.data();
		pipelineData.descriptorSetLayouts = descriptorSetLayouts.data();
	}

	graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}

void* GraphicsPipelineImporter::LoadAsset(Uuid uuid) {
	auto& pipelineIterator = assets.emplace(uuid, GraphicsPipelineAsset(uuid));
	GraphicsPipelineAsset& graphicsPipelineAsset = pipelineIterator.first->second;

	graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportGraphicsPipelineAsset(graphicsPipelineAsset)) {
		return nullptr;
	}

	return &graphicsPipelineAsset;
}

void GraphicsPipelineImporter::QueueReloadAsset(Uuid uuid) {
	auto& shaderInMap = assets.find(uuid);
	if (shaderInMap == assets.end()) {
		return;
	}

	Grindstone::GraphicsPipelineAsset& graphicsPipelineAsset = shaderInMap->second;

	graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Reloading;
	Grindstone::GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsPipelineAsset.passes.clear();
	graphicsPipelineAsset.metaData.resources.clear();
	graphicsPipelineAsset.metaData.buffers.clear();
	ImportGraphicsPipelineAsset(graphicsPipelineAsset);
}

GraphicsPipelineImporter::~GraphicsPipelineImporter() {
	assets.clear();
}
