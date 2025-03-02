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
		return false;
	}

	std::vector<GraphicsPipeline::CreateInfo::ShaderStageData> shaderStageCreateInfos;
	std::vector<std::vector<char>> fileData;

	graphicsPipelineAsset.name = result.displayName;

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

		pipelineCreateInfo.primitiveType = GeometryType::Triangles;
		pipelineCreateInfo.polygonFillMode = GraphicsAPI::PolygonFillMode::Fill;
		pipelineCreateInfo.cullMode = CullMode::None;
		pipelineCreateInfo.shaderStageCreateInfos = shaderStageCreateInfos.data();
		pipelineCreateInfo.shaderStageCreateInfoCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
		pipelineCreateInfo.descriptorSetLayouts = nullptr;
		pipelineCreateInfo.descriptorSetLayoutCount = static_cast<uint32_t>(0);
		pipelineCreateInfo.vertexBindings = nullptr;
		pipelineCreateInfo.vertexBindingsCount = 0;
		pipelineCreateInfo.colorAttachmentCount = 0;
		pipelineCreateInfo.blendData = BlendData::NoBlending();
		pipelineCreateInfo.isDepthTestEnabled = true;
		pipelineCreateInfo.isDepthWriteEnabled = true;
		pipelineCreateInfo.isStencilEnabled = false;
		pass.pipeline = graphicsCore->CreateGraphicsPipeline(pipelineCreateInfo);

		// TODO: Fix
		pass.descriptorSetLayouts[0] = pipelineCreateInfo.descriptorSetLayouts[0];
	}

	// TODO: Save compiled shader into ShaderCache

	graphicsPipelineAsset.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}

void* GraphicsPipelineImporter::LoadAsset(Uuid uuid) {
	auto asset = assets.emplace(uuid, GraphicsPipelineAsset(uuid));
	GraphicsPipelineAsset& graphicsPipelineAsset = asset.first->second;

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
