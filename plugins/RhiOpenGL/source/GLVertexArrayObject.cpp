#include <GL/gl3w.h>

#include <RhiOpenGL/include/GLBuffer.hpp>
#include <RhiOpenGL/include/GLFormats.hpp>
#include <RhiOpenGL/include/GLVertexArrayObject.hpp>

using namespace Grindstone::GraphicsAPI;

OpenGL::VertexArrayObject::VertexArrayObject() {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
}

OpenGL::VertexArrayObject::VertexArrayObject(const VertexArrayObject::CreateInfo& createInfo) : GraphicsAPI::VertexArrayObject(createInfo.layout) {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	if (createInfo.debugName != nullptr) {
		glObjectLabel(GL_VERTEX_ARRAY, vertexArrayObject, -1, createInfo.debugName);
	}

	size_t vertexBufferCount = min(static_cast<uint32_t>(createInfo.vertexBufferCount), layout.bindings.size());
	for (size_t i = 0; i < vertexBufferCount; ++i) {
		OpenGL::Buffer* vbo = static_cast<OpenGL::Buffer*>(createInfo.vertexBuffers[i]);
		const VertexBindingDescription& binding = layout.bindings[i];

		GLuint divisor = binding.inputRate == VertexInputRate::Vertex ? 0 : 1;
		GLintptr bindingOffset = 0; // NOTE: I don't think this works in Vulkan, so we're not supporting it.

		glVertexArrayBindingDivisor(vertexArrayObject, binding.bindingIndex, divisor);
		glVertexArrayVertexBuffer(
			vertexArrayObject,
			binding.bindingIndex,
			vbo->GetBuffer(),
			bindingOffset,
			binding.stride
		);
	}

	for (uint32_t j = 0; j < layout.attributes.size(); ++j) {
		const VertexAttributeDescription& attribute = layout.attributes[j];

		OpenGLFormats oglFormat = TranslateFormatToOpenGL(attribute.format);
		const GLenum isNormalized = true; // TODO: FormatTypeComponents(attribute.format);
		const GLenum componentCount = 4; // TODO: FormatTypeComponents(attribute.format);
		const GLuint offset = attribute.byteOffset;

		glEnableVertexArrayAttrib(vertexArrayObject, attribute.locationIndex);
		glVertexArrayAttribBinding(vertexArrayObject, attribute.locationIndex, attribute.bindingIndex);
		glVertexArrayAttribFormat(
			vertexArrayObject,
			attribute.locationIndex,
			componentCount,
			oglFormat.format,
			isNormalized,
			offset
		);
	}

	if (createInfo.indexBuffer != nullptr) {
		OpenGL::Buffer* glIndexBuffer = static_cast<OpenGL::Buffer*>(createInfo.indexBuffer);
		glVertexArrayElementBuffer(vertexArrayObject, glIndexBuffer->GetBuffer());
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
