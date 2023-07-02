#include <assert.h>
#include "VulkanUniformBuffer.hpp"
#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

using namespace Grindstone::GraphicsAPI;

VulkanUniformBuffer::VulkanUniformBuffer(UniformBuffer::CreateInfo& createInfo) {
	size = createInfo.size;
	CreateBuffer(
		size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffer,
		memory
	);
}

VulkanUniformBuffer::~VulkanUniformBuffer() {
	VkDevice device = VulkanCore::Get().GetDevice();

	if (buffer != nullptr) {
		vkDestroyBuffer(device, buffer, nullptr);
	}

	if (memory != nullptr) {
		vkFreeMemory(device, memory, nullptr);
	}
}
		
void VulkanUniformBuffer::UpdateBuffer(void * content) {
	VkDevice device = VulkanCore::Get().GetDevice();
	void* data;
	vkMapMemory(device, memory, 0, size, 0, &data);
	memcpy(data, content, size);
	vkUnmapMemory(device, memory);
}

uint32_t VulkanUniformBuffer::GetSize() {
	return size;
}

VkBuffer VulkanUniformBuffer::GetBuffer() {
	return buffer;
}

void VulkanUniformBuffer::Bind() {
	std::cout << "VulkanUniformBuffer::Bind is not used.\n";
	assert(false);
}
