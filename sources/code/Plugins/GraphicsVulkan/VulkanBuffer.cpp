#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"
#include "VulkanBuffer.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::Buffer::Buffer(const CreateInfo& createInfo) : GraphicsAPI::Buffer(createInfo) {
	VkBufferUsageFlags usage = 0;
	switch (bufferUsage) {
	case BufferUsage::Vertex:		usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;		break;
	case BufferUsage::Index:		usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;		break;
	case BufferUsage::Uniform:		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;		break;
	case BufferUsage::Storage:		usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;		break;
	case BufferUsage::Indirect:		usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;	break;
	case BufferUsage::TransferSrc:	usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;		break;
	case BufferUsage::TransferDst:	usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;		break;
	}

	VkMemoryPropertyFlags properties = 0;
	switch (memoryUsage) {
	case MemUsage::GPUOnly:		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; break;
	case MemUsage::CPUOnly:		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
	case MemUsage::CPUToGPU:	properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
	case MemUsage::GPUToCPU:	properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT; break;
	}

	VkDevice device = Vulkan::Core::Get().GetDevice();
	CreateBuffer(createInfo.debugName, bufferSize, usage, properties, bufferObject, deviceMemory);

	// Copy data if content is not nullptr.
	if (createInfo.content != nullptr) {
		if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) > 0) {
			// If I'm CPUOnly or CPUToGPU, then copy data directly to buffer.
			void* data;
			vkMapMemory(device, deviceMemory, 0, bufferSize, 0, &data);
			memcpy(data, createInfo.content, bufferSize);
			vkUnmapMemory(device, deviceMemory);
		}
		else {
			// Otherwise, use a staging buffer.
			std::string stagingDebugName = std::string(createInfo.debugName) + " Staging";

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			CreateBuffer(stagingDebugName.c_str(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, createInfo.content, bufferSize);
			vkUnmapMemory(device, stagingBufferMemory);

			CopyBuffer(stagingBuffer, bufferObject, bufferSize);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);
		}
	}
}

Vulkan::Buffer::~Buffer() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	if (bufferObject != nullptr) {
		vkDestroyBuffer(device, bufferObject, nullptr);
	}

	if (deviceMemory != nullptr) {
		vkFreeMemory(device, deviceMemory, nullptr);
	}
}

void* Vulkan::Buffer::Map() {
	if (!mappedMemoryPtr) {
		VkDevice device = Vulkan::Core::Get().GetDevice();
		vkMapMemory(device, deviceMemory, 0, bufferSize, 0, &mappedMemoryPtr);
	}

	return mappedMemoryPtr;
}

void Vulkan::Buffer::Unmap() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	if (mappedMemoryPtr) {
		vkUnmapMemory(device, deviceMemory);
		mappedMemoryPtr = nullptr;
	}
}

void Vulkan::Buffer::UploadData(const void* data, size_t size, size_t offset) {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	bool ownsMemoryMap = false;
	if (!mappedMemoryPtr) {
		ownsMemoryMap = true;
		vkMapMemory(device, deviceMemory, 0, bufferSize, 0, &mappedMemoryPtr);
	}

	std::memcpy(mappedMemoryPtr, data, size);

	if (ownsMemoryMap) {
		vkUnmapMemory(device, deviceMemory);
	}
}

VkBuffer Vulkan::Buffer::GetBuffer() const {
	return bufferObject;
}
