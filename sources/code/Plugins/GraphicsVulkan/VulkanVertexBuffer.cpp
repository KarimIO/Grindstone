#include "VulkanVertexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

namespace Grindstone::GraphicsAPI {
	VulkanVertexBuffer::VulkanVertexBuffer(VertexBuffer::CreateInfo& createInfo) {
		VkDevice device = VulkanCore::Get().GetDevice();

		VkDeviceSize bufferSize = createInfo.size;

		std::string stagingDebugName = std::string(createInfo.debugName) + " Staging";

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(stagingDebugName.c_str(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, createInfo.content, (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		CreateBuffer(createInfo.debugName, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

		CopyBuffer(stagingBuffer, buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer() {
		VkDevice device = VulkanCore::Get().GetDevice();
		vkDestroyBuffer(device, buffer, nullptr);
		vkFreeMemory(device, memory, nullptr);
	}

	VkBuffer VulkanVertexBuffer::GetBuffer() const {
		return buffer;
	}
}
