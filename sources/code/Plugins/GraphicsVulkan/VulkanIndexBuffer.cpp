#include "VulkanIndexBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanIndexBuffer::VulkanIndexBuffer(IndexBuffer::CreateInfo& createInfo) {
			is32Bit = createInfo.is32Bit;

			VkDevice device = VulkanCore::Get().GetDevice();

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

		VulkanIndexBuffer::~VulkanIndexBuffer() {
			VkDevice device = VulkanCore::Get().GetDevice();
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, memory, nullptr);
		}

		VkBuffer VulkanIndexBuffer::GetBuffer() {
			return buffer;
		}

		bool VulkanIndexBuffer::Is32Bit() {
			return is32Bit;
		}
	}
}

