#pragma once

#include "../GraphicsCommon/UniformBuffer.h"
#include <vulkan/vulkan.h>


class vkUniformBufferBinding : public UniformBufferBinding {
	VkDescriptorSetLayout descriptorSetLayout;
	VkDevice *device;
public:
	vkUniformBufferBinding(VkDevice *device, UniformBufferBindingCreateInfo);
	VkDescriptorSetLayout *getLayout();
	~vkUniformBufferBinding();
};

class vkUniformBuffer : public UniformBuffer {
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	VkDescriptorPool *descriptorPool;

	VkBuffer buffer;
	VkDeviceMemory memory;

	uint32_t size;

	VkDescriptorSet descriptorSet;
public:
	vkUniformBuffer(VkDevice *dev, VkPhysicalDevice *phys, VkDescriptorPool *descPool, UniformBufferCreateInfo ci);
	void UpdateUniformBuffer(void * content);
	VkDescriptorSet *GetDescriptorSet();
	~vkUniformBuffer();
};