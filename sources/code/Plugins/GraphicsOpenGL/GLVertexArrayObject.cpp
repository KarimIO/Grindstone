#include <GL/gl3w.h>
#include "GLVertexArrayObject.hpp"

namespace Grindstone::GraphicsAPI
{
	GLenum vertexFormatToOpenGLFormat(VertexFormat format) {
		switch (format)
		{
		case VertexFormat::Float:
		case VertexFormat::Float2:
		case VertexFormat::Float3:
		case VertexFormat::Float4:
		case VertexFormat::Mat3:
		case VertexFormat::Mat4:	return GL_FLOAT;

		case VertexFormat::Int:
		case VertexFormat::Int2:
		case VertexFormat::Int3:
		case VertexFormat::Int4:	return GL_INT;

		case VertexFormat::UInt:
		case VertexFormat::UInt2:
		case VertexFormat::UInt3:
		case VertexFormat::UInt4:	return GL_UNSIGNED_INT;
		case VertexFormat::Bool:	return GL_BOOL;
		}

		return 0;
	};

	GLVertexArrayObject::GLVertexArrayObject() {
		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);
	}

	GLVertexArrayObject::GLVertexArrayObject(CreateInfo& createInfo) {
		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);
		if (createInfo.debugName != nullptr) {
			glObjectLabel(GL_VERTEX_ARRAY, vertexArrayObject, -1, createInfo.debugName);
		}

		for (size_t i = 0; i < createInfo.vertexBufferCount; ++i) {
			GLVertexBuffer* vbo = static_cast<GLVertexBuffer*>(createInfo.vertexBuffers[i]);
			vbo->Bind();

			const auto& layout = vbo->GetLayout();
			for (uint32_t j = 0; j < layout.attributeCount; ++j) {
				const VertexAttributeDescription &layoutElement = layout.attributes[j];
				const GLenum vertexFormat = vertexFormatToOpenGLFormat(layoutElement.format);
				glEnableVertexAttribArray(layoutElement.location);
				glVertexAttribPointer(
					layoutElement.location,
					layoutElement.componentsCount,
					vertexFormat,
					layoutElement.isNormalized ? GL_TRUE : GL_FALSE,
					layout.stride,
					reinterpret_cast<const void*>(layoutElement.offset)
				);
			}
		}

		if (createInfo.indexBuffer != nullptr) {
			static_cast<GLIndexBuffer*>(createInfo.indexBuffer)->Bind();
		}

		glBindVertexArray(0);
	}

	GLVertexArrayObject::~GLVertexArrayObject() {
		glDeleteVertexArrays(1, &vertexArrayObject);
	}

	void GLVertexArrayObject::Bind() {
		glBindVertexArray(vertexArrayObject);
	}

	void GLVertexArrayObject::Unbind() {
		glBindVertexArray(0);
	}
}
