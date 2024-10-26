#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"
#include "VulkanUniformBuffer.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::UniformBuffer::UniformBuffer(const CreateInfo& createInfo) {
	size = createInfo.size;
	CreateBuffer(
		createInfo.debugName,
		size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		buffer,
		memory
	);
}

Vulkan::UniformBuffer::~UniformBuffer() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	if (buffer != nullptr) {
		vkDestroyBuffer(device, buffer, nullptr);
	}

	if (memory != nullptr) {
		vkFreeMemory(device, memory, nullptr);
	}
}
		
void Vulkan::UniformBuffer::UpdateBuffer(void * content) {
	VkDevice device = Vulkan::Core::Get().GetDevice();
	void* data;
	vkMapMemory(device, memory, 0, size, 0, &data);
	memcpy(data, content, size);
	vkUnmapMemory(device, memory);
}

uint32_t Vulkan::UniformBuffer::GetSize() const {
	return size;
}

VkBuffer Vulkan::UniformBuffer::GetBuffer() const {
	return buffer;
}

void Vulkan::UniformBuffer::Bind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanUniformBuffer::Bind is not used.");
	assert(false);
}
