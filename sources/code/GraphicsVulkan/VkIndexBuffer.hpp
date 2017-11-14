#pragma once

#include <vulkan/vulkan.h>
#include "VkGraphicsWrapper.h"
#include "../GraphicsCommon/IndexBuffer.h"

class VkGraphicsWrapper;

class vkIndexBuffer : public IndexBuffer {
public:
	VkGraphicsWrapper *graphicsWrapper;
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	uint32_t size;
	uint32_t count;

	VkBuffer buffer;
	VkDeviceMemory memory;
public:
	vkIndexBuffer(VkGraphicsWrapper *, VkDevice *dev, VkPhysicalDevice *physicalDevice, IndexBufferCreateInfo createInfo);
	~vkIndexBuffer();

	VkBuffer *GetBuffer();
	uint32_t GetSize();
	uint32_t GetCount();
};