#pragma once

#include <Common/Graphics/VertexBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(VertexBuffer::CreateInfo& createInfo);
			virtual ~VulkanVertexBuffer() override;
		public:
			VkBuffer GetBuffer() const;
		private:
			VkBuffer buffer = nullptr;
			VkDeviceMemory memory = nullptr;
		};
	};
};
