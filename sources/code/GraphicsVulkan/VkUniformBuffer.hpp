#pragma once

#include "../GraphicsCommon/UniformBuffer.hpp"
#include <vulkan/vulkan.h>


class VulkanUniformBufferBinding : public UniformBufferBinding {
	VkDescriptorSetLayout descriptorSetLayout;
	VkDevice *device;
public:
	VulkanUniformBufferBinding(VkDevice *device, UniformBufferBindingCreateInfo);
	VkDescriptorSetLayout *getLayout();
	~VulkanUniformBufferBinding();
};

class VulkanUniformBuffer : public UniformBuffer {
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	VkDescriptorPool *descriptorPool;

	VkBuffer buffer;
	VkDeviceMemory memory;

	uint32_t size;

	VkDescriptorSet descriptorSet;
public:
	VulkanUniformBuffer(VkDevice *dev, VkPhysicalDevice *phys, VkDescriptorPool *descPool, UniformBufferCreateInfo ci);
	void UpdateUniformBuffer(void * content);
	VkDescriptorSet *GetDescriptorSet();
	~VulkanUniformBuffer();
};