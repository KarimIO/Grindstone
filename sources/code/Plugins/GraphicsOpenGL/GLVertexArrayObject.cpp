#include <GL/gl3w.h>

#include "GLFormats.hpp"
#include "GLVertexArrayObject.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::VertexArrayObject::VertexArrayObject() {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
}

OpenGL::VertexArrayObject::VertexArrayObject(const VertexArrayObject::CreateInfo& createInfo) {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	if (createInfo.debugName != nullptr) {
		glObjectLabel(GL_VERTEX_ARRAY, vertexArrayObject, -1, createInfo.debugName);
	}

	for (size_t i = 0; i < createInfo.vertexBufferCount; ++i) {
		OpenGL::VertexBuffer* vbo = static_cast<OpenGL::VertexBuffer*>(createInfo.vertexBuffers[i]);
		vbo->Bind();

		const auto& layout = vbo->GetLayout();
		for (uint32_t j = 0; j < layout.attributeCount; ++j) {
			const VertexAttributeDescription &layoutElement = layout.attributes[j];
			const GLenum vertexFormat = TranslateVertexFormatToOpenGL(layoutElement.format);
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
		static_cast<OpenGL::IndexBuffer*>(createInfo.indexBuffer)->Bind();
	}

	glBindVertexArray(0);
}

OpenGL::VertexArrayObject::~VertexArrayObject() {
	glDeleteVertexArrays(1, &vertexArrayObject);
}

void OpenGL::VertexArrayObject::Bind() {
	glBindVertexArray(vertexArrayObject);
}

void OpenGL::VertexArrayObject::Unbind() {
	glBindVertexArray(0);
}
