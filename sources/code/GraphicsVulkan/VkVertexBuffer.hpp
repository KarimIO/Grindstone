#pragma once

#include <vulkan/vulkan.h>
#include "../GraphicsCommon/VertexBuffer.hpp"
#include "vkGraphicsWrapper.hpp"

class VulkanGraphicsWrapper;

class VulkanVertexBuffer : public VertexBuffer {
public:
	VulkanGraphicsWrapper *graphicsWrapper;
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	uint32_t size;
	uint32_t count;

	VkBuffer buffer;
	VkDeviceMemory memory;
public:
	VulkanVertexBuffer(VulkanGraphicsWrapper *, VkDevice *dev, VkPhysicalDevice *physicalDevice, VertexBufferCreateInfo createInfo);
	~VulkanVertexBuffer();

	VkBuffer *GetBuffer();
	uint32_t GetSize();
	uint32_t GetCount();
};