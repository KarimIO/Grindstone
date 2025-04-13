#include <filesystem>

#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Rendering/DeferredRenderer.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>

#include "GraphicsPipelineImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;

struct GraphicsPipelineFileHeader {
	float depthBiasClamp;
	float depthBiasConstantFactor;
	float depthBiasSlopeFactor;
	Grindstone::GraphicsAPI::CullMode cullMode;
	Grindstone::GraphicsAPI::CompareOperation depthCompareOp;
	Grindstone::GraphicsAPI::PolygonFillMode polygonFillMode;
	Grindstone::GraphicsAPI::GeometryType primitiveType;
	uint8_t flags;
	uint8_t attachmentCount;
	uint8_t shaderStageCount;
};


struct GraphicsPipelineShaderStageHeaders {
	uint8_t stageType;
	uint32_t shaderCodeSize;
};

struct GraphicsPipelineAttachmentFileHeader {
	Grindstone::GraphicsAPI::ColorMask colorMask;
	Grindstone::GraphicsAPI::BlendFactor blendAlphaFactorDst;
	Grindstone::GraphicsAPI::BlendFactor blendAlphaFactorSrc;
	Grindstone::GraphicsAPI::BlendOperation blendAlphaOperation;
	Grindstone::GraphicsAPI::BlendFactor blendColorFactorDst;
	Grindstone::GraphicsAPI::BlendFactor blendColorFactorSrc;
	Grindstone::GraphicsAPI::BlendOperation blendColorOperation;
};

static void UnpackGraphicsPipelineHeader(const GraphicsPipelineFileHeader& srcHeader, GraphicsPipeline::CreateInfo& dstHeader) {
	dstHeader.primitiveType = srcHeader.primitiveType;
	dstHeader.polygonFillMode = srcHeader.polygonFillMode;
	dstHeader.depthCompareOp = srcHeader.depthCompareOp;
	dstHeader.cullMode = srcHeader.cullMode;
	dstHeader.depthBiasClamp = srcHeader.depthBiasClamp;
	dstHeader.depthBiasConstantFactor = srcHeader.depthBiasConstantFactor;
	dstHeader.depthBiasSlopeFactor = srcHeader.depthBiasSlopeFactor;

	dstHeader.isDepthBiasEnabled = srcHeader.flags & 0b1;
	dstHeader.isDepthClampEnabled = srcHeader.flags & 0b10;
	dstHeader.isDepthTestEnabled = srcHeader.flags & 0b100;
	dstHeader.isDepthWriteEnabled = srcHeader.flags & 0b1000;
	dstHeader.isStencilEnabled = srcHeader.flags & 0b10000;

	// TODO: We should check this and probably apply the first attachment data to others if necessary.
	// dstHeader.shouldCopyFirstAttachment ? 0b100000;
}

static void UnpackGraphicsPipelineAttachmentHeader(
	const GraphicsPipelineAttachmentFileHeader& srcHeader,
	Grindstone::GraphicsAPI::GraphicsPipeline::CreateInfo::AttachmentData& dstHeader
) {
	dstHeader.colorMask = srcHeader.colorMask;
	dstHeader.blendData.alphaFactorDst = srcHeader.blendAlphaFactorDst;
	dstHeader.blendData.alphaFactorSrc = srcHeader.blendAlphaFactorSrc;
	dstHeader.blendData.alphaOperation = srcHeader.blendAlphaOperation;
	dstHeader.blendData.colorFactorDst = srcHeader.blendColorFactorDst;
	dstHeader.blendData.colorFactorSrc = srcHeader.blendColorFactorSrc;
	dstHeader.blendData.colorOperation = srcHeader.blendColorOperation;
}

// Graphics Pipeline Format:
// GGP + null-terminated character magic text
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

	if (memcmp(fileData.Get(), "GGP\0", 4) != 0) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Graphics Pipeline file does not start with GGP - {}.", result.displayName);
		graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	graphicsPipelineAsset.name = result.displayName;

	unsigned char* readPtr = fileData.Get() + 4;
	GraphicsPipelineFileHeader* srcFileHeader = reinterpret_cast<GraphicsPipelineFileHeader*>(readPtr);
	readPtr += sizeof(GraphicsPipelineFileHeader);

	GraphicsPipelineShaderStageHeaders* srcStageHeaders = reinterpret_cast<GraphicsPipelineShaderStageHeaders*>(readPtr);
	readPtr += sizeof(GraphicsPipelineShaderStageHeaders) * srcFileHeader->shaderStageCount;

	GraphicsPipelineAttachmentFileHeader* srcAttachmentHeaders = reinterpret_cast<GraphicsPipelineAttachmentFileHeader*>(readPtr);
	readPtr += sizeof(GraphicsPipelineAttachmentFileHeader) * srcFileHeader->attachmentCount;

	std::vector<GraphicsPipeline::CreateInfo::ShaderStageData> shaderStageCreateInfos;
	shaderStageCreateInfos.resize(srcFileHeader->shaderStageCount);

	graphicsPipelineAsset.passes.resize(1);
	for (GraphicsPipelineAsset::Pass& pass : graphicsPipelineAsset.passes) {
		GraphicsPipeline::CreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.debugName = result.displayName.c_str();
		pipelineCreateInfo.width = 0.0f;
		pipelineCreateInfo.height = 0.0f;
		pipelineCreateInfo.scissorX = 0;
		pipelineCreateInfo.scissorY = 0;
		pipelineCreateInfo.scissorW = 0;
		pipelineCreateInfo.scissorH = 0;
		pipelineCreateInfo.hasDynamicViewport = true;
		pipelineCreateInfo.hasDynamicScissor = true;

		GraphicsAPI::RenderPass* renderPass = nullptr; // TODO: FindRenderPass(renderStage);
		pipelineCreateInfo.renderPass = renderPass;

		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = nullptr;
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(0);
		pipelineCreateInfo.vertexBindings = nullptr;
		pipelineCreateInfo.vertexBindingsCount = 0;
		pipelineCreateInfo.colorAttachmentCount = 0;

		std::vector<GraphicsPipelineAttachmentFileHeader> srcAttachmentHeaders;
		srcAttachmentHeaders.resize(srcAttachmentHeaders.size());
		for (size_t i = 0; i < srcAttachmentHeaders.size(); ++i) {
			UnpackGraphicsPipelineAttachmentHeader(srcAttachmentHeaders[i], pipelineCreateInfo.colorAttachmentData[i]);
		}

		UnpackGraphicsPipelineHeader(*srcFileHeader, pipelineCreateInfo);

		for (size_t i = 0; i < srcFileHeader->shaderStageCount; ++i) {
			shaderStageCreateInfos[i].content = reinterpret_cast<const char*>(readPtr);
			shaderStageCreateInfos[i].size = srcStageHeaders[i].shaderCodeSize;
			shaderStageCreateInfos[i].type = static_cast<Grindstone::GraphicsAPI::ShaderStage>(srcStageHeaders[i].stageType);

			readPtr += srcStageHeaders[i].shaderCodeSize;
		}

		pass.pipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);

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
	for (Grindstone::GraphicsPipelineAsset::Pass& pass : graphicsPipelineAsset.passes) {
		graphicsCore->DeleteGraphicsPipeline(pass.pipeline);
	}

	graphicsPipelineAsset.passes.clear();
	ImportGraphicsPipelineAsset(graphicsPipelineAsset);
}

GraphicsPipelineImporter::~GraphicsPipelineImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		for (Grindstone::GraphicsPipelineAsset::Pass& pass : asset.second.passes) {
			graphicsCore->DeleteGraphicsPipeline(pass.pipeline);
		}
	}
	assets.clear();
}
