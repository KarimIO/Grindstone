#pragma once

#include <string>
#include <vector>
#include <map>
#include <entt/fwd.hpp>
#include <glm/vec3.hpp>

#include <Common/HashedString.hpp>
#include <Common/Rendering/GeometryRenderingStats.hpp>
#include <Common/Rendering/RenderViewData.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

using RenderQueueIndex = uint8_t;
const RenderQueueIndex INVALID_RENDER_QUEUE = UINT8_MAX;

namespace Grindstone {
	class WorldContextSet;

	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	class BaseAssetRenderer {
	public:
		virtual std::string GetName() const = 0;
		virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) = 0;
		virtual Grindstone::Rendering::GeometryRenderStats RenderQueue(
			GraphicsAPI::CommandBuffer* commandBuffer,
			const Grindstone::Rendering::RenderViewData& viewData,
			Grindstone::WorldContextSet* cxtSet,
			Grindstone::HashedString renderQueueHash
		) = 0;
	};
}
