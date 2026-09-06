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
			const std::string& passName,
			GraphicsAPI::CommandBuffer* commandBuffer,
			Grindstone::Rendering::AssetRenderQueueContext& cxt
		);
		
		std::map<std::string, BaseAssetRenderer*> assetRenderers;
	};
}
