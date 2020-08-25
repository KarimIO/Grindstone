#pragma once

#include <Common/Graphics/UniformBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanUniformBufferBinding : public UniformBufferBinding {
		public:
			VulkanUniformBufferBinding(UniformBufferBinding::CreateInfo& ci);
			virtual ~VulkanUniformBufferBinding();
		public:
			VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptor_set_layout_;
		};

		class VulkanUniformBuffer : public UniformBuffer {
		public:
			VulkanUniformBuffer(UniformBuffer::CreateInfo& ci);
			virtual ~VulkanUniformBuffer();
		public:
			VkDescriptorSet getDescriptorSet();
		public:
			virtual void updateBuffer(void * content) override;
			virtual void bind() override;
		private:
			VkDescriptorSet descriptor_set_;
			VkBuffer buffer_;
			VkDeviceMemory memory_;
			uint32_t size_;
		};
	};
};
