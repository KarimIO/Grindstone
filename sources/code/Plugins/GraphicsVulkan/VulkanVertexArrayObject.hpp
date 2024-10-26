#pragma once

#include <vector>

#include <Common/Graphics/VertexArrayObject.hpp>

namespace Grindstone::GraphicsAPI::Vulkan {
	class VertexBuffer;
	class IndexBuffer;

	class VertexArrayObject : public Grindstone::GraphicsAPI::VertexArrayObject {
	public:
		VertexArrayObject(const CreateInfo& createInfo);

		// Inherited from VertexArrayObject
		virtual ~VertexArrayObject() override;
		virtual void Bind() override;
		virtual void Unbind() override;
	public:
		std::vector<GraphicsAPI::VertexBuffer*>& GetVertexBuffers();
		GraphicsAPI::IndexBuffer* GetIndexBuffer() const;
	private:
		std::vector<GraphicsAPI::VertexBuffer*> vertexBuffers;
		GraphicsAPI::IndexBuffer* indexBuffer = nullptr;
	};
}
