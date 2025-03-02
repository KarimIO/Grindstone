#include <filesystem>

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

static bool ImportComputeAsset(ComputePipelineAsset& computePipelineAsset) {
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

	std::vector<GraphicsPipeline::CreateInfo::ShaderStageData> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

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
	auto asset = assets.emplace(uuid, ComputePipelineAsset(uuid));
	ComputePipelineAsset& computePipelineAsset = asset.first->second;

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
