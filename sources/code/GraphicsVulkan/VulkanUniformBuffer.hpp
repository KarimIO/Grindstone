#pragma once

#include "../GraphicsCommon/UniformBuffer.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanUniformBufferBinding : public UniformBufferBinding {
		public:
			VulkanUniformBufferBinding(UniformBufferBindingCreateInfo ci);
			virtual ~VulkanUniformBufferBinding();
		public:
			VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptor_set_layout_;
		};

		class VulkanUniformBuffer : public UniformBuffer {
		public:
			VulkanUniformBuffer(UniformBufferCreateInfo ci);
			virtual ~VulkanUniformBuffer();
		public:
			VkDescriptorSet getDescriptorSet();
		public:
			virtual void UpdateUniformBuffer(void * content) override;
			virtual void Bind() override;
		private:
			VkDescriptorSet descriptor_set_;
			VkBuffer buffer_;
			VkDeviceMemory memory_;
			uint32_t size_;
		};
	};
};
