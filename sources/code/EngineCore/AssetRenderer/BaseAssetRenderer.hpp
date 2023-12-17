#pragma once

#include <string>
#include <vector>
#include <map>
#include <entt/fwd.hpp>

#include "EngineCore/Assets/AssetManager.hpp"

using RenderQueueIndex = uint8_t;
const RenderQueueIndex INVALID_RENDER_QUEUE = UINT8_MAX;

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	enum class DrawSortMode {
		DistanceFrontToBack,
		DistanceBackToFront
	};

	class BaseAssetRenderer {
	public:
		virtual void AddQueue(const char* name, DrawSortMode sortType) = 0;
		virtual void RenderShadowMap(GraphicsAPI::CommandBuffer* commandBuffer, GraphicsAPI::DescriptorSet* lightingDescriptorSet) = 0;
		virtual void CacheRenderTasksAndFrustumCull(GraphicsAPI::CommandBuffer* commandBuffer, entt::registry& registry) = 0;

		virtual std::string GetName() const = 0;
		virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) = 0;
		virtual void RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, const char* queueName) = 0;
	};
}
