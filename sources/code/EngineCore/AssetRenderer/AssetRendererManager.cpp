#include "AssetRendererManager.hpp"
#include "EngineCore/Profiling.hpp"
#include "EngineCore/EngineCore.hpp"
using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers[assetRenderer->GetName()] = assetRenderer;

	for (const char* name : assetQueuesNames) {
		assetRenderer->AddQueue(name);
	}
}

void AssetRendererManager::AddQueue(const char* name) {
	assetQueuesNames.emplace_back(name);

	for (auto assetRenderer : assetRenderers) {
		assetRenderer.second->AddQueue(name);
	}
}

void AssetRendererManager::RegisterShader(std::string& geometryType, std::string& renderQueue, Uuid shaderUuid) {
	auto assetRenderer = assetRenderers.find(geometryType);
	if (assetRenderer == assetRenderers.end()) {
		Grindstone::EngineCore::GetInstance().Print(LogSeverity::Error, "Unable to register shader.");
		return;
	}

	assetRenderer->second->AddShaderToRenderQueue(shaderUuid, renderQueue.c_str());
}

void AssetRendererManager::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->SetEngineDescriptorSet(descriptorSet);
	}
}

void AssetRendererManager::RenderShadowMap(
	GraphicsAPI::CommandBuffer* commandBuffer,
	GraphicsAPI::DescriptorSet* lightingDescriptorSet
) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderShadowMap(commandBuffer, lightingDescriptorSet);
	}
}

void AssetRendererManager::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	const char* name
) {
	std::string profileScope = std::string("AssetRendererManager::RenderQueue(") + name + ")";
	GRIND_PROFILE_SCOPE(profileScope.c_str());
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderQueue(commandBuffer, name);
	}
}

void AssetRendererManager::RenderQueueImmediate(const char* name) {
	std::string profileScope = std::string("AssetRendererManager::RenderQueue(") + name + ")";
	GRIND_PROFILE_SCOPE(profileScope.c_str());
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderQueueImmediate(name);
	}
}
