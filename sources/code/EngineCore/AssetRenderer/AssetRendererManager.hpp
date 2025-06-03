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
		virtual void AddAssetRenderer(BaseAssetRenderer* assetRenderer);
		virtual void RemoveAssetRenderer(BaseAssetRenderer* assetRenderer);
		virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet);
		virtual void RenderQueue(
			GraphicsAPI::CommandBuffer* commandBuffer,
			const entt::registry& registry,
			Grindstone::HashedString renderQueue
		);
		
		std::map<std::string, BaseAssetRenderer*> assetRenderers;
	};
}
