#include <Common/Graphics/CommandBuffer.hpp>
#include <Common/Rendering/RenderViewData.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>

#include "AssetRendererManager.hpp"

using namespace Grindstone;

void AssetRendererManager::AddAssetRenderer(BaseAssetRenderer* assetRenderer) {
	assetRenderers[assetRenderer->GetName()] = assetRenderer;
}

void AssetRendererManager::RemoveAssetRenderer(BaseAssetRenderer* assetRenderer) {
	std::string assetRendererName = assetRenderer->GetName();
	auto rendererInMap = assetRenderers.find(assetRendererName.c_str());
	if (rendererInMap != assetRenderers.end()) {
		assetRenderers.erase(rendererInMap);
	}
	else {
		GPRINT_ERROR_V(Grindstone::LogSource::RenderingBackend, "Unable to find asset renderer to remove", assetRendererName.c_str());
	}
}

void AssetRendererManager::SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) {
	for (auto& assetRenderer : assetRenderers) {
		assetRenderer.second->SetEngineDescriptorSet(descriptorSet);
	}
}

Grindstone::Rendering::GeometryRenderStats AssetRendererManager::RenderQueue(
	GraphicsAPI::CommandBuffer* commandBuffer,
	const Grindstone::Rendering::RenderViewData& viewData,
	entt::registry& registry,
	Grindstone::HashedString renderQueue
) {
	Grindstone::Rendering::GeometryRenderStats stats{};

	std::string renderQueueLabel = std::vformat("Render Queue '{}'", std::make_format_args(renderQueue.ToString()));
	commandBuffer->BeginDebugLabelSection(renderQueueLabel.c_str());
	for (auto& assetRenderer : assetRenderers) {
		Grindstone::Rendering::GeometryRenderStats currentStats = assetRenderer.second->RenderQueue(commandBuffer, viewData, registry, renderQueue);

		stats.drawCalls += currentStats.drawCalls;
		stats.triangles += currentStats.triangles;
		stats.vertices += currentStats.vertices;
		stats.objectsCulled += currentStats.objectsCulled;
		stats.objectsRendered += currentStats.objectsRendered;
		stats.pipelineBinds += currentStats.pipelineBinds;
		stats.materialBinds += currentStats.materialBinds;

		stats.gpuTimeMs += currentStats.gpuTimeMs;
		stats.cpuTimeMs += currentStats.cpuTimeMs;
	}

	commandBuffer->EndDebugLabelSection();

	stats.renderQueue = renderQueue;

	return stats;
}
