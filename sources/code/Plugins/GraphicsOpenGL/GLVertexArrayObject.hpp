#pragma once

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/VertexArrayObject.hpp>

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
		VertexInputLayout layout;
	};
}
