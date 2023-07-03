#include <iostream>
#include <assert.h>

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

		IndexBuffer* VulkanVertexArrayObject::GetIndexBuffer() {
			return indexBuffer;
		}

		void VulkanVertexArrayObject::Bind() {
			std::cout << "VulkanVertexArrayObject::Bind is not used.\n";
			assert(false);
		}

		void VulkanVertexArrayObject::Unbind() {
			std::cout << "VulkanVertexArrayObject::Bind is not used.\n";
			assert(false);
		}
	}
}
