#include <Common/Graphics/CommandBuffer.hpp>
#include "EngineCore/Profiling.hpp"
#include "EngineCore/EngineCore.hpp"

#include "AssetRendererManager.hpp"

using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers[assetRenderer->GetName()] = assetRenderer;
}

void AssetRendererManager::RemoveAssetRenderer(BaseAssetRenderer* assetRenderer) {
	auto& rendererInMap = assetRenderers.find(assetRenderer->GetName());
	if (rendererInMap == assetRenderers.end()) {
		assetRenderers.erase(rendererInMap);
	}
}

void AssetRendererManager::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->SetEngineDescriptorSet(descriptorSet);
	}
}

void AssetRendererManager::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	entt::registry& registry,
	Grindstone::HashedString renderQueue
) {
	std::string renderQueueLabel = fmt::format("Render Queue '{}'", renderQueue.ToString());
	commandBuffer->BeginDebugLabelSection(renderQueueLabel.c_str());
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->RenderQueue(commandBuffer, registry, renderQueue);
	}
	commandBuffer->EndDebugLabelSection();
}
