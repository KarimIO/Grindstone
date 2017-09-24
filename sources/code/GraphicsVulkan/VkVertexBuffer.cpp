#include "VkVertexBuffer.h"
#include <array>
#include "VkBufferCommon.h"
#include <cstring>

vkVertexBuffer::vkVertexBuffer(VkGraphicsWrapper *gw, VkDevice *dev, VkPhysicalDevice *physical, VertexBufferCreateInfo createInfo) {
	device = dev;
	graphicsWrapper = gw;
	physicalDevice = physical;
	size = createInfo.size;
	count = createInfo.count;

	VkDeviceSize bufferSize = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
	std::memcpy(data, createInfo.content, (size_t)bufferSize);
	vkUnmapMemory(*device, stagingBufferMemory);

	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

	gw->copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

VkBuffer * vkVertexBuffer::GetBuffer() {
	return &buffer;
}

uint32_t vkVertexBuffer::GetSize() {
	return size;
}

uint32_t vkVertexBuffer::GetCount() {
	return count;
}

vkVertexBuffer::~vkVertexBuffer() {
	vkDestroyBuffer(*device, buffer, nullptr);
	vkFreeMemory(*device, memory, nullptr);
}
