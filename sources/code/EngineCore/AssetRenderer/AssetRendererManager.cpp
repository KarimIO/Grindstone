#include <Common/Graphics/CommandBuffer.hpp>
#include "EngineCore/Profiling.hpp"
#include "EngineCore/EngineCore.hpp"

#include "AssetRendererManager.hpp"

using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers[assetRenderer->GetName()] = assetRenderer;

	for (std::string& queueName : assetQueuesNames) {
		assetRenderer->AddQueue(queueName.c_str(), queueDrawSortModes[queueName]);
	}
}

void AssetRendererManager::RemoveAssetRenderer(BaseAssetRenderer* assetRenderer) {
	auto& rendererInMap = assetRenderers.find(assetRenderer->GetName());
	if (rendererInMap == assetRenderers.end()) {
		assetRenderers.erase(rendererInMap);
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
	GraphicsAPI::DescriptorSet* lightingDescriptorSet,
	entt::registry& registry,
	glm::vec3 lightSourcePosition
) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderShadowMap(commandBuffer, lightingDescriptorSet, registry, lightSourcePosition);
	}
}

void AssetRendererManager::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	const char* queueName
) {
	std::string profileScope = std::string("AssetRendererManager::RenderQueue(") + queueName + ")";
	GRIND_PROFILE_SCOPE(profileScope.c_str());
	commandBuffer->BeginDebugLabelSection(profileScope.c_str());
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderQueue(commandBuffer, queueName);
	}
	commandBuffer->EndDebugLabelSection();
}

void AssetRendererManager::CacheRenderTasksAndFrustumCull(glm::vec3 eyePosition, entt::registry& registry) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->CacheRenderTasksAndFrustumCull(eyePosition, registry);
	}
}

void AssetRendererManager::SortQueues() {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->SortQueues();
	}
}
