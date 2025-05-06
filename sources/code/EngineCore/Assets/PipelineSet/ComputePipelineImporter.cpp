#include <filesystem>

#include <Common/Formats/PipelineSet.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Rendering/DeferredRenderer.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>

#include "ComputePipelineImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::GraphicsAPI;
using namespace Grindstone::Formats::Pipelines;

static bool ImportComputeAsset(ComputePipelineAsset& computePipelineAsset) {
	return false;
	// TODO: Check shader cache before loading and compiling again
	// The shader cache includes shaders precompiled for consoles, or compiled once per driver update on computers
	// if shaderCache has shader with this uuid
	//		return shader

	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore.assetManager;
	AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;

	Assets::AssetLoadBinaryResult result = assetManager->LoadBinaryByUuid(AssetType::ComputePipelineSet, computePipelineAsset.uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find shader with id {}.", computePipelineAsset.uuid.ToString());
		return false;
	}

	Grindstone::Buffer& fileData = result.buffer;
	GS_ASSERT(fileData.GetCapacity() >= (4 + sizeof(V1::PipelineSetFileHeader)));

	if (memcmp(fileData.Get(), V1::FileMagicCode, 4) != 0) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Graphics Pipeline file does not start with GPSF - {}.", result.displayName);
		computePipelineAsset.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	computePipelineAsset.name = result.displayName;

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

	std::vector<GraphicsPipeline::ShaderStageData> shaderStageCreateInfos;

	computePipelineAsset.name = result.displayName;

	ComputePipeline::CreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.debugName = result.displayName.c_str();
	pipelineCreateInfo.descriptorSetLayouts = nullptr;
	pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(0);
	pipelineCreateInfo.shaderContent;
	pipelineCreateInfo.shaderFileName;
	pipelineCreateInfo.shaderSize;
	computePipelineAsset.pipeline = graphicsCore->CreateComputePipeline(pipelineCreateInfo);

	// TODO: Fix
	computePipelineAsset.descriptorSetLayouts[0] = pipelineCreateInfo.descriptorSetLayouts[0];

	// TODO: Save compiled shader into ShaderCache

	computePipelineAsset.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}

void* ComputePipelineImporter::LoadAsset(Uuid uuid) {
	auto& pipelineIterator = assets.emplace(uuid, ComputePipelineAsset(uuid));
	ComputePipelineAsset& computePipelineAsset = pipelineIterator.first->second;

	computePipelineAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportComputeAsset(computePipelineAsset)) {
		return nullptr;
	}

	return &computePipelineAsset;
}

void ComputePipelineImporter::QueueReloadAsset(Uuid uuid) {
	auto& shaderInMap = assets.find(uuid);
	if (shaderInMap == assets.end()) {
		return;
	}

	Grindstone::ComputePipelineAsset& computePipelineAsset = shaderInMap->second;

	computePipelineAsset.assetLoadStatus = AssetLoadStatus::Reloading;
	Grindstone::GraphicsAPI::Core* graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	graphicsCore->DeleteComputePipeline(computePipelineAsset.pipeline);

	ImportComputeAsset(computePipelineAsset);
}

ComputePipelineImporter::~ComputePipelineImporter() {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();

	for (auto& asset : assets) {
		graphicsCore->DeleteComputePipeline(asset.second.pipeline);
	}
	assets.clear();
}
