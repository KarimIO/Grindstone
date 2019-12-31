#include "VkIndexBuffer.hpp"
#include <array>
#include "VkBufferCommon.hpp"
#include <cstring>

vkIndexBuffer::vkIndexBuffer(vkGraphicsWrapper *gw, VkDevice *dev, VkPhysicalDevice *physical, IndexBufferCreateInfo createInfo) {
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

	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

	gw->copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

VkBuffer * vkIndexBuffer::GetBuffer() {
	return &buffer;
}

uint32_t vkIndexBuffer::GetSize() {
	return size;
}

uint32_t vkIndexBuffer::GetCount() {
	return count;
}

vkIndexBuffer::~vkIndexBuffer() {
	vkDestroyBuffer(*device, buffer, nullptr);
	vkFreeMemory(*device, memory, nullptr);
}
