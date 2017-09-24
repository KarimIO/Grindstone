#pragma once

#include <vulkan/vulkan.h>
#include "../GraphicsCommon/VertexBuffer.h"
#include "VkGraphicsWrapper.h"

class VkGraphicsWrapper;

class vkVertexBuffer : public VertexBuffer {
public:
	VkGraphicsWrapper *graphicsWrapper;
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	uint32_t size;
	uint32_t count;

	VkBuffer buffer;
	VkDeviceMemory memory;
public:
	vkVertexBuffer(VkGraphicsWrapper *, VkDevice *dev, VkPhysicalDevice *physicalDevice, VertexBufferCreateInfo createInfo);
	~vkVertexBuffer();

	VkBuffer *GetBuffer();
	uint32_t GetSize();
	uint32_t GetCount();
};