#pragma once

#include <Common/Graphics/IndexBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanIndexBuffer : public IndexBuffer {
		public:
			VulkanIndexBuffer(IndexBuffer::CreateInfo& ci);
			virtual ~VulkanIndexBuffer() override;
		public:
			VkBuffer getBuffer();
		private:
			VkBuffer buffer_;
			VkDeviceMemory memory_;
		};
	}
}