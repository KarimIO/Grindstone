#pragma once

#include <vector>
#include <map>
#include <entt/fwd.hpp>

#include <Common/Rendering/RenderViewData.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>

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
		virtual Grindstone::Rendering::GeometryRenderStats RenderQueue(
			GraphicsAPI::CommandBuffer* commandBuffer,
			const Grindstone::Rendering::RenderViewData& viewData,
			entt::registry& registry,
			Grindstone::HashedString renderQueue
		);
		
		std::map<std::string, BaseAssetRenderer*> assetRenderers;
	};
}
