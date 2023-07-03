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
			bool Is32Bit();
		private:
			VkBuffer buffer = nullptr;
			VkDeviceMemory memory = nullptr;
			bool is32Bit = false;
		};
	}
}
