#pragma once

#include <cstdint>

#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <Common/Rect.hpp>
#include <Common/Rendering/RenderViewData.hpp>

#include "TransientResourceManager.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace GraphicsAPI {
		class Core;
		class CommandBuffer;
		class DescriptorSet;
	}

	namespace Renderer {
		struct RenderGraphContext {
			Grindstone::GraphicsAPI::Core* graphicsCore = nullptr;
			Grindstone::Rendering::RenderViewData cameraViewData;
			Grindstone::Renderer::TransientResourceManager* transientResourceManager = nullptr;
			Grindstone::GraphicsAPI::DescriptorSetLayoutRef globalDescriptorSetLayout;
			Grindstone::GraphicsAPI::DescriptorSet* globalDescriptorSet = nullptr;
			Grindstone::Math::Extent2D swapchainSize;
			Grindstone::GraphicsAPI::CommandBuffer* commandBuffer = nullptr;
			Grindstone::WorldContextSet* worldContextSet = nullptr;
			uint32_t swapchainIndex = 0;
		};
	}
}
