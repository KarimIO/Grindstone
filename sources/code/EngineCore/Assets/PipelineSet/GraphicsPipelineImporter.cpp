#include <filesystem>

#include <Common/Containers/Span.hpp>
#include <Common/Formats/PipelineSet.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Rendering/DeferredRenderer.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>

#include "GraphicsPipelineImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Containers;
using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Formats::Pipelines;

static void UnpackGraphicsPipelineHeader(
	const V1::PassPipelineHeader& srcHeader,
	GraphicsPipeline::PipelineData& dstPipelineData
) {
	dstPipelineData.primitiveType = srcHeader.primitiveType;
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
	std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout*>& dstDescriptorSets
) {
	Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo layoutCreateInfo;

	for (uint8_t i = 0; i < srcDescriptorSets.GetSize(); ++i) {
		const V1::ShaderReflectDescriptorSet& srcDescriptorSet = srcDescriptorSets[i];

		std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> dstDescriptorBindings;
		dstDescriptorBindings.reserve(srcDescriptorSet.bindingCount);

		for (size_t bindingIndex = srcDescriptorSet.firstBindingIndex;
			bindingIndex < srcDescriptorSet.firstBindingIndex + srcDescriptorSet.bindingCount;
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

		dstDescriptorSets[i] = graphicsCore->CreateDescriptorSetLayout(layoutCreateInfo);
	}
}

// Graphics Pipeline Format:
// GPSF character magic text
// GraphicsPipelineFileHeader
// GraphicsPipelineShaderStageHeaders[]
// GraphicsPipelineAttachmentFileHeader[]
// Shader Code[]

static bool ImportGraphicsPipelineAsset(GraphicsPipelineAsset& graphicsPipelineAsset) {
	// TODO: Check shader cache before loading and compiling again
	// The shader cache includes shaders precompiled for consoles, or compiled once per driver update on computers
	// if shaderCache has shader with this uuid
	//		return shader

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore.assetManager;
	AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;

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

	unsigned char* readPtr = fileData.Get() + 4;
	V1::PipelineSetFileHeader* srcFileHeader = reinterpret_cast<V1::PipelineSetFileHeader*>(readPtr);
	readPtr += sizeof(V1::PipelineSetFileHeader);

	GS_ASSERT(srcFileHeader->headerSize == sizeof(V1::PipelineSetFileHeader));
	GS_ASSERT(srcFileHeader->graphicsPipelineSize == sizeof(V1::GraphicsPipelineHeader));
	GS_ASSERT(srcFileHeader->computePipelineSize == sizeof(V1::ComputePipelineHeader));
	GS_ASSERT(srcFileHeader->configurationSize == sizeof(V1::PipelineConfigurationHeader));
	GS_ASSERT(srcFileHeader->passSize == sizeof(V1::PassPipelineHeader));
	GS_ASSERT(srcFileHeader->attachmentSize == sizeof(V1::PassPipelineAttachmentHeader));
	GS_ASSERT(srcFileHeader->stageSize == sizeof(V1::PassPipelineShaderStageHeader));

	V1::GraphicsPipelineHeader* srcPipelineHeader = reinterpret_cast<V1::GraphicsPipelineHeader*>(readPtr);
	readPtr += sizeof(V1::GraphicsPipelineHeader) * srcFileHeader->graphicsPipelineCount;

	// TODO: Do something with Compute Pipelines
	readPtr += sizeof(V1::ComputePipelineHeader) * srcFileHeader->computePipelineCount;

	V1::PipelineConfigurationHeader* srcConfigHeader = reinterpret_cast<V1::PipelineConfigurationHeader*>(readPtr);
	readPtr += sizeof(V1::PipelineConfigurationHeader) * srcPipelineHeader->configurationCount;

	// TODO: Loop over Configurations
	graphicsPipelineAsset.passes.resize(srcConfigHeader->passCount);
	for (uint32_t passIndex = 0; passIndex < srcConfigHeader->passCount; ++passIndex) {
		V1::PassPipelineHeader* srcPassHeader = reinterpret_cast<V1::PassPipelineHeader*>(readPtr);
		readPtr += sizeof(V1::PassPipelineHeader);

		Span<V1::PassPipelineShaderStageHeader> srcStageHeaders{ reinterpret_cast<V1::PassPipelineShaderStageHeader*>(readPtr), srcPassHeader->shaderStageCount };
		readPtr += sizeof(V1::PassPipelineShaderStageHeader) * srcPassHeader->shaderStageCount;

		Span<V1::PassPipelineAttachmentHeader> srcAttachmentHeaders{ reinterpret_cast<V1::PassPipelineAttachmentHeader*>(readPtr), srcPassHeader->attachmentCount };
		readPtr += sizeof(V1::PassPipelineAttachmentHeader) * srcPassHeader->attachmentCount;

		Span<V1::ShaderReflectDescriptorSet> srcDescriptorSets{ reinterpret_cast<V1::ShaderReflectDescriptorSet*>(readPtr), srcPassHeader->descriptorSetCount };
		readPtr += sizeof(V1::ShaderReflectDescriptorSet) * srcPassHeader->descriptorSetCount;

		Span<V1::ShaderReflectDescriptorBinding> srcDescriptorBindings{ reinterpret_cast<V1::ShaderReflectDescriptorBinding*>(readPtr), srcPassHeader->descriptorBindingCount };
		readPtr += sizeof(V1::ShaderReflectDescriptorBinding) * srcPassHeader->descriptorBindingCount;

		GraphicsPipelineAsset::Pass& pass = graphicsPipelineAsset.passes[passIndex];

		std::vector<GraphicsPipeline::ShaderStageData> shaderStageCreateInfos;
		shaderStageCreateInfos.resize(srcPassHeader->shaderStageCount);

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
		pipelineData.renderPass = nullptr; // TODO: FindRenderPass(renderStage);

		pipelineData.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineData.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineData.descriptorSetLayoutCount = static_cast<uint32_t>(srcPassHeader->descriptorSetCount);
		pipelineData.colorAttachmentCount = srcPassHeader->attachmentCount;

		UnpackGraphicsPipelineHeader(*srcPassHeader, pipelineData);

		for (uint8_t i = 0; i < srcPassHeader->shaderStageCount; ++i) {
			shaderStageCreateInfos[i].content = reinterpret_cast<const char*>(readPtr);
			shaderStageCreateInfos[i].size = srcStageHeaders[i].shaderCodeSize;
			shaderStageCreateInfos[i].type = static_cast<Grindstone::GraphicsAPI::ShaderStage>(srcStageHeaders[i].stageType);

			readPtr += srcStageHeaders[i].shaderCodeSize;
		}

		std::vector<Grindstone::GraphicsAPI::DescriptorSetLayout*> descriptorSetLayouts;
		descriptorSetLayouts.resize(srcPassHeader->descriptorSetCount);
		UnpackGraphicsPipelineAttachmentHeaders(srcAttachmentHeaders, pipelineData.colorAttachmentData);
		UnpackGraphicsPipelineDescriptorSetHeaders(
			graphicsCore,
			graphicsPipelineAsset.name,
			srcDescriptorSets,
			srcDescriptorBindings,
			descriptorSetLayouts
		);
		pipelineData.descriptorSetLayouts = descriptorSetLayouts.data();

		// TODO: Fix
		// pass.descriptorSetLayouts[0] = pipelineCreateInfo.descriptorSetLayouts[0];
	}

	// TODO: Save compiled shader into ShaderCache

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
	ImportGraphicsPipelineAsset(graphicsPipelineAsset);
}

GraphicsPipelineImporter::~GraphicsPipelineImporter() {
	assets.clear();
}
