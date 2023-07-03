#pragma once

#include <vector>

#include "BaseAssetRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
		class Framebuffer;
		class CommandBuffer;
	}

	class AssetRendererManager {
	public:
		void AddAssetRenderer(BaseAssetRenderer* assetRenderer);
		void AddQueue(const char* name);
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
