#pragma once

#include <Common/Graphics/VertexArrayObject.hpp>

#include "GLVertexBuffer.hpp"
#include "GLIndexBuffer.hpp"

namespace Grindstone::GraphicsAPI::OpenGL {
	class VertexArrayObject : public Grindstone::GraphicsAPI::VertexArrayObject {
	public:
		VertexArrayObject();
		VertexArrayObject(const CreateInfo& createInfo);
		virtual ~VertexArrayObject() override;

		virtual void Bind() override;
		virtual void Unbind() override;

	protected:
		GLuint vertexArrayObject;
		uint32_t vertexBufferCount = 0;

	};
}
