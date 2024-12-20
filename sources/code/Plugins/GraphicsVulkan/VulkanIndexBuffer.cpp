#include "VulkanIndexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::IndexBuffer::IndexBuffer(const CreateInfo& createInfo) {
	is32Bit = createInfo.is32Bit;

	VkDevice device = Vulkan::Core::Get().GetDevice();

	VkDeviceSize bufferSize = createInfo.size;

	std::string debugName = std::string(createInfo.debugName) + " Staging";

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(createInfo.debugName, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, createInfo.content, (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(createInfo.debugName, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

	CopyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

Vulkan::IndexBuffer::~IndexBuffer() {
	VkDevice device = Vulkan::Core::Get().GetDevice();
	vkDestroyBuffer(device, buffer, nullptr);
	vkFreeMemory(device, memory, nullptr);
}

VkBuffer Vulkan::IndexBuffer::GetBuffer() const {
	return buffer;
}

bool Vulkan::IndexBuffer::Is32Bit() const {
	return is32Bit;
}

