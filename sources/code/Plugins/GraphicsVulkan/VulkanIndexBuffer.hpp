#pragma once

#include "../GraphicsCommon/IndexBuffer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanIndexBuffer : public IndexBuffer {
		public:
			VulkanIndexBuffer(IndexBufferCreateInfo ci);
			virtual ~VulkanIndexBuffer() override;
		public:
			VkBuffer getBuffer();
		private:
			VkBuffer buffer_;
			VkDeviceMemory memory_;
		};
	}
}