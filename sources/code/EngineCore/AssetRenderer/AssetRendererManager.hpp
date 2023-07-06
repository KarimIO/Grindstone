#pragma once

#include <vector>

#include "BaseAssetRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	class AssetRendererManager {
	public:
		void AddAssetRenderer(BaseAssetRenderer* assetRenderer);
		void AddQueue(const char* name);
		void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet);
		void RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet);
		void RenderQueue(
			GraphicsAPI::CommandBuffer* commandBuffer,
			const char* name
		);
		void RenderQueueImmediate(const char* name);
	private:
		std::vector<BaseAssetRenderer*> assetRenderers;
		std::vector<const char*> assetQueuesNames;
	};
}
