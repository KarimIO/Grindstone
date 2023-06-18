#pragma once

#include <Common/Graphics/UniformBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanUniformBufferBinding : public UniformBufferBinding {
		public:
			VulkanUniformBufferBinding(UniformBufferBinding::CreateInfo& createInfo);
			virtual ~VulkanUniformBufferBinding();
		public:
			VkDescriptorSetLayout GetDescriptorSetLayout();
			uint32_t GetBinding();
		private:
			VkDescriptorSetLayout descriptorSetLayout = nullptr;
			uint32_t binding = -1;
		};

		class VulkanUniformBuffer : public UniformBuffer {
		public:
			VulkanUniformBuffer(UniformBuffer::CreateInfo& createInfo);
			virtual ~VulkanUniformBuffer();
		public:
			VkDescriptorSet GetDescriptorSet();
		public:
			virtual void UpdateBuffer(void *content) override;
			virtual void Bind() override;
		private:
			VkDescriptorSet descriptorSet = nullptr;
			VkDeviceMemory memory = nullptr;
			VkBuffer buffer = nullptr;
			uint32_t size = 0;
		};
	};
};
