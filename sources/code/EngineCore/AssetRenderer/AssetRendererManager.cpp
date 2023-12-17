#include "AssetRendererManager.hpp"
#include "EngineCore/Profiling.hpp"
#include "EngineCore/EngineCore.hpp"
using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers[assetRenderer->GetName()] = assetRenderer;

	for (std::string& queueName : assetQueuesNames) {
		assetRenderer->AddQueue(queueName.c_str(), queueDrawSortModes[queueName]);
	}
}

void AssetRendererManager::AddQueue(const char* name, DrawSortMode sortMode) {
	assetQueuesNames.emplace_back(name);
	queueDrawSortModes[name] = sortMode;

	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->AddQueue(name, sortMode);
	}
}

void AssetRendererManager::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->SetEngineDescriptorSet(descriptorSet);
	}
}

RenderQueueIndex Grindstone::AssetRendererManager::GetIndexOfRenderQueue(const std::string& renderQueue) const {
	for (RenderQueueIndex i = 0; i < assetQueuesNames.size(); ++i) {
		if (renderQueue == assetQueuesNames[i]) {
			return i;
		}
	}

	return INVALID_RENDER_QUEUE;
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
	const char* queueName
) {
	std::string profileScope = std::string("AssetRendererManager::RenderQueue(") + queueName + ")";
	GRIND_PROFILE_SCOPE(profileScope.c_str());
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderQueue(commandBuffer, queueName);
	}
}

void Grindstone::AssetRendererManager::CacheRenderTasksAndFrustumCull(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->CacheRenderTasksAndFrustumCull(commandBuffer, registry);
	}
}
