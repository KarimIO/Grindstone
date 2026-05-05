#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/HashedString.hpp>
#include <Common/Rect.hpp>

#include "TransientResourceManager.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	namespace Renderer {
		struct RenderGraphContext {
			Grindstone::Renderer::TransientResourceManager* transientResourceManager = nullptr;
			Grindstone::GraphicsAPI::DescriptorSet* globalDescriptorSet = nullptr;
			Grindstone::Math::Extent2D swapchainSize;
			Grindstone::GraphicsAPI::CommandBuffer* commandBuffer = nullptr;
			Grindstone::WorldContextSet* worldContextSet = nullptr;
			uint32_t swapchainIndex = 0;
		};
	}
}
