#pragma once

#include <Common/Graphics/VertexBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(VertexBuffer::CreateInfo& ci);
			virtual ~VulkanVertexBuffer() override;
		public:
			VkBuffer getBuffer();
		private:
			VkBuffer buffer_;
			VkDeviceMemory memory_;
		};
	};
};