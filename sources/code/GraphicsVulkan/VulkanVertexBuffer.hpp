#pragma once

#include "../GraphicsCommon/VertexBuffer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(VertexBufferCreateInfo ci);
			virtual ~VulkanVertexBuffer() {};
		public:
			VkBuffer getBuffer();
		private:
			VkBuffer buffer_;
			VkDeviceMemory memory_;
		};
	};
};