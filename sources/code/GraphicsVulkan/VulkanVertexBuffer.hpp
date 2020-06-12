#pragma once

#include "../GraphicsCommon/VertexBuffer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(VertexBufferCreateInfo ci);
			virtual ~VulkanVertexBuffer() override;
		public:
			virtual void updateBuffer(void* content) override;
			VkBuffer getBuffer();
		private:
			VkBuffer buffer_;
			VkDeviceMemory memory_;
			uint32_t size_;
		};
	};
};