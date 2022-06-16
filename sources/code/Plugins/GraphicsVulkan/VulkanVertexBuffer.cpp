#include "VulkanVertexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanVertexBuffer::VulkanVertexBuffer(VertexBuffer::CreateInfo& ci) {
			VkDevice device = VulkanCore::Get().GetDevice();

			VkDeviceSize bufferSize = ci.size;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, ci.content, (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_, memory_);

			CopyBuffer(stagingBuffer, buffer_, bufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}

		VulkanVertexBuffer::~VulkanVertexBuffer() {
			VkDevice device = VulkanCore::Get().GetDevice();
			vkDestroyBuffer(device, buffer_, nullptr);
			vkFreeMemory(device, memory_, nullptr);
		}

		VkBuffer VulkanVertexBuffer::getBuffer() {
			return buffer_;
		}
	}
}
