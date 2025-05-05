#pragma once

#include <vector>

#include <Common/Graphics/VertexArrayObject.hpp>

namespace Grindstone::GraphicsAPI::Vulkan {
	class Buffer;

	class VertexArrayObject : public Grindstone::GraphicsAPI::VertexArrayObject {
	public:
		VertexArrayObject(const CreateInfo& createInfo);

		// Inherited from VertexArrayObject
		virtual ~VertexArrayObject() override;
		virtual void Bind() override;
		virtual void Unbind() override;

	public:
		std::vector<GraphicsAPI::Buffer*>& GetVertexBuffers();
		GraphicsAPI::Buffer* GetIndexBuffer() const;

	private:
		std::vector<GraphicsAPI::Buffer*> vertexBuffers;
		GraphicsAPI::Buffer* indexBuffer = nullptr;
		GraphicsAPI::VertexInputLayout layout;
	};
}
