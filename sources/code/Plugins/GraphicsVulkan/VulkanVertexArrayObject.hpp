#pragma once

#include <vector>

#include <Common/Graphics/VertexArrayObject.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanVertexBuffer;
		class VulkanIndexBuffer;

		class VulkanVertexArrayObject : public VertexArrayObject {
		public:
			VulkanVertexArrayObject(VertexArrayObject::CreateInfo& createInfo);

			// Inherited from VertexArrayObject
			virtual ~VulkanVertexArrayObject() override;
			virtual void Bind() override;
			virtual void Unbind() override;
		public:
			std::vector<VertexBuffer*>& GetVertexBuffers();
			IndexBuffer* GetIndexBuffer();
		private:
			std::vector<VertexBuffer*> vertexBuffers;
			IndexBuffer* indexBuffer = nullptr;
		};
	};
};
