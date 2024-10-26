#include <iostream>
#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanVertexArrayObject.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanIndexBuffer.hpp"

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::VertexArrayObject::VertexArrayObject(const CreateInfo& createInfo) {
	vertexBuffers.resize(createInfo.vertexBufferCount);

	for (uint32_t i = 0; i < createInfo.vertexBufferCount; ++i) {
		vertexBuffers[i] = static_cast<Vulkan::VertexBuffer*>(createInfo.vertexBuffers[i]);
	}

	indexBuffer = static_cast<Vulkan::IndexBuffer*>(createInfo.indexBuffer);
}

Vulkan::VertexArrayObject::~VertexArrayObject() {
}

std::vector<Base::VertexBuffer*>& Vulkan::VertexArrayObject::GetVertexBuffers() {
	return vertexBuffers;
}

Base::IndexBuffer* Vulkan::VertexArrayObject::GetIndexBuffer() const  {
	return indexBuffer;
}

void Vulkan::VertexArrayObject::Bind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanVertexArrayObject::Bind is not used.");
}

void Vulkan::VertexArrayObject::Unbind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanVertexArrayObject::Bind is not used.");
}
