#pragma once

#include <Common/Graphics/UniformBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanUniformBuffer : public UniformBuffer {
		public:
			VulkanUniformBuffer(UniformBuffer::CreateInfo& createInfo);
			virtual ~VulkanUniformBuffer();

			virtual void UpdateBuffer(void* content) override;
			virtual uint32_t GetSize() override;

			virtual VkBuffer GetBuffer();
		private:
			VkDeviceMemory memory = nullptr;
			VkBuffer buffer = nullptr;
			uint32_t size = 0;
		};
	};
};
