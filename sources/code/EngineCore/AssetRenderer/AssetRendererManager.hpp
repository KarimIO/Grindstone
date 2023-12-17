#pragma once

#include <vector>
#include <map>
#include <entt/fwd.hpp>

#include "BaseAssetRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	class AssetRendererManager {
	public:
		void AddAssetRenderer(BaseAssetRenderer* assetRenderer);
		void AddQueue(const char* name, DrawSortMode sortType);
		void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet);
		RenderQueueIndex GetIndexOfRenderQueue(const std::string& renderQueue) const;
		void RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet);
		void RenderQueue(
			GraphicsAPI::CommandBuffer* commandBuffer,
			const char* name
		);
		void CacheRenderTasksAndFrustumCull(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry);
	private:
		std::map<std::string, BaseAssetRenderer*> assetRenderers;
		std::map<std::string, DrawSortMode> queueDrawSortModes;
		std::vector<std::string> assetQueuesNames;
	};
}
