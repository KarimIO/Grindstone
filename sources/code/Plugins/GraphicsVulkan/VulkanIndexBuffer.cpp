#include "VulkanIndexBuffer.hpp"
#include "VulkanGraphicsWrapper.hpp"
#include "VulkanUtils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanIndexBuffer::VulkanIndexBuffer(IndexBufferCreateInfo ci) {
			VkDevice device = VulkanGraphicsWrapper::get().getDevice();

			VkDeviceSize bufferSize = ci.size;

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, ci.content, (size_t)bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer_, memory_);

			copyBuffer(stagingBuffer, buffer_, bufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}

		VulkanIndexBuffer::~VulkanIndexBuffer() {
			VkDevice device = VulkanGraphicsWrapper::get().getDevice();
			vkDestroyBuffer(device, buffer_, nullptr);
			vkFreeMemory(device, memory_, nullptr);
		}

		VkBuffer VulkanIndexBuffer::getBuffer() {
			return buffer_;
		}
	}
}

