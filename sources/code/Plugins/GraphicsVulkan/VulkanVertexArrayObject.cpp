#include <iostream>
#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanVertexArrayObject.hpp"
#include "VulkanVertexBuffer.hpp"
#include "VulkanIndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanVertexArrayObject::VulkanVertexArrayObject(VertexArrayObject::CreateInfo& createInfo) {
			vertexBuffers.resize(createInfo.vertexBufferCount);

			for (uint32_t i = 0; i < createInfo.vertexBufferCount; ++i) {
				vertexBuffers[i] = createInfo.vertexBuffers[i];
			}

			indexBuffer = static_cast<VulkanIndexBuffer*>(createInfo.indexBuffer);
		}

		VulkanVertexArrayObject::~VulkanVertexArrayObject() {
		}

		std::vector<VertexBuffer*>& VulkanVertexArrayObject::GetVertexBuffers() {
			return vertexBuffers;
		}

		IndexBuffer* VulkanVertexArrayObject::GetIndexBuffer() const  {
			return indexBuffer;
		}

		void VulkanVertexArrayObject::Bind() {
			GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanVertexArrayObject::Bind is not used.");
		}

		void VulkanVertexArrayObject::Unbind() {
			GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanVertexArrayObject::Bind is not used.");
		}
	}
}
