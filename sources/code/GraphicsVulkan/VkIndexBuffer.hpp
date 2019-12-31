#pragma once

#include <vulkan/vulkan.h>
#include "vkGraphicsWrapper.hpp"
#include "../GraphicsCommon/IndexBuffer.hpp"

class VulkanGraphicsWrapper;

class VulkanIndexBuffer : public IndexBuffer {
public:
	VulkanGraphicsWrapper *graphicsWrapper;
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	uint32_t size;
	uint32_t count;

	VkBuffer buffer;
	VkDeviceMemory memory;
public:
	VulkanIndexBuffer(VulkanGraphicsWrapper *, VkDevice *dev, VkPhysicalDevice *physicalDevice, IndexBufferCreateInfo createInfo);
	~VulkanIndexBuffer();

	VkBuffer *GetBuffer();
	uint32_t GetSize();
	uint32_t GetCount();
};