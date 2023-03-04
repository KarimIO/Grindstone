#pragma once

#include <vector>

#include "BaseAssetRenderer.hpp"

namespace Grindstone {
	class AssetRendererManager {
	public:
		void AddAssetRenderer(BaseAssetRenderer* assetRenderer);
		void AddQueue(const char* name);
		void RenderQueue(const char* name);
	private:
		std::vector<BaseAssetRenderer*> assetRenderers;
		std::vector<const char*> assetQueuesNames;
	};
}
