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
			VkDescriptorSetLayout GetDescriptorSetLayout();
		private:
			VkDescriptorSetLayout descriptorSetLayout;
		};

		class VulkanUniformBuffer : public UniformBuffer {
		public:
			VulkanUniformBuffer(UniformBuffer::CreateInfo& ci);
			virtual ~VulkanUniformBuffer();
		public:
			VkDescriptorSet GetDescriptorSet();
		public:
			virtual void UpdateBuffer(void *content) override;
			virtual void Bind() override;
		private:
			VkDescriptorSet descriptorSet;
			VkBuffer buffer;
			VkDeviceMemory memory;
			uint32_t size;
		};
	};
};
