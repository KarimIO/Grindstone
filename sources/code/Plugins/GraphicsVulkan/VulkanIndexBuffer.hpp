#pragma once

#include <Common/Graphics/IndexBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanIndexBuffer : public IndexBuffer {
		public:
			VulkanIndexBuffer(IndexBuffer::CreateInfo& createInfo);
			virtual ~VulkanIndexBuffer() override;
		public:
			VkBuffer GetBuffer();
		private:
			VkBuffer buffer = nullptr;
			VkDeviceMemory memory = nullptr;
		};
	}
}
