#include <iostream>
#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanVertexArrayObject.hpp"
#include "VulkanBuffer.hpp"

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::VertexArrayObject::VertexArrayObject(const CreateInfo& createInfo) : Base::VertexArrayObject(createInfo.layout) {
	vertexBuffers.resize(createInfo.vertexBufferCount);

	for (uint32_t i = 0; i < createInfo.vertexBufferCount; ++i) {
		vertexBuffers[i] = static_cast<Vulkan::Buffer*>(createInfo.vertexBuffers[i]);
	}

	indexBuffer = static_cast<Vulkan::Buffer*>(createInfo.indexBuffer);
}

Vulkan::VertexArrayObject::~VertexArrayObject() {
}

const std::vector<Base::Buffer*>& Vulkan::VertexArrayObject::GetVertexBuffers() const {
	return vertexBuffers;
}

Base::Buffer* Vulkan::VertexArrayObject::GetIndexBuffer() const  {
	return indexBuffer;
}

void Vulkan::VertexArrayObject::Bind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::VertexArrayObject::Bind is not used.");
}

void Vulkan::VertexArrayObject::Unbind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Vulkan::VertexArrayObject::Unbind is not used.");
}
