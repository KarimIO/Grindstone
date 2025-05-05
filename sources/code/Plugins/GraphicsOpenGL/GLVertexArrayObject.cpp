#include <GL/gl3w.h>

#include "GLBuffer.hpp"
#include "GLFormats.hpp"
#include "GLVertexArrayObject.hpp"

using namespace Grindstone::GraphicsAPI;

static uint32_t VertexFormatTypeSize(VertexFormat type) {
	switch (type) {
	case VertexFormat::Float:    return 4;
	case VertexFormat::Float2:   return 4 * 2;
	case VertexFormat::Float3:   return 4 * 3;
	case VertexFormat::Float4:   return 4 * 4;
	case VertexFormat::Mat3:     return 4 * 3 * 3;
	case VertexFormat::Mat4:     return 4 * 4 * 4;
	case VertexFormat::Int:      return 4;
	case VertexFormat::Int2:     return 4 * 2;
	case VertexFormat::Int3:     return 4 * 3;
	case VertexFormat::Int4:     return 4 * 4;
	case VertexFormat::UInt:     return 4;
	case VertexFormat::UInt2:    return 4 * 2;
	case VertexFormat::UInt3:    return 4 * 3;
	case VertexFormat::UInt4:    return 4 * 4;
	case VertexFormat::Bool:     return 1;
	}

	return 0;
};

static uint32_t VertexFormatTypeComponents(VertexFormat type) {
	switch (type) {
	case VertexFormat::Float:    return 1;
	case VertexFormat::Float2:   return 2;
	case VertexFormat::Float3:   return 3;
	case VertexFormat::Float4:   return 4;
	case VertexFormat::Mat3:     return 4 * 3;
	case VertexFormat::Mat4:     return 4 * 4;
	case VertexFormat::Int:      return 1;
	case VertexFormat::Int2:     return 2;
	case VertexFormat::Int3:     return 3;
	case VertexFormat::Int4:     return 4;
	case VertexFormat::UInt:     return 1;
	case VertexFormat::UInt2:    return 2;
	case VertexFormat::UInt3:    return 3;
	case VertexFormat::UInt4:    return 4;
	case VertexFormat::Bool:     return 1;
	}

	return 0;
};

static GLenum VertexFormatTypeComponents(VertexFormat type) {
	// TODO: Implement this

	return GL_FALSE;
};

OpenGL::VertexArrayObject::VertexArrayObject() {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
}

OpenGL::VertexArrayObject::VertexArrayObject(const VertexArrayObject::CreateInfo& createInfo) : layout(createInfo.layout) {
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

		const GLenum vertexFormat = TranslateVertexFormatToOpenGL(attribute.format);
		const GLenum isNormalized = VertexFormatTypeComponents(attribute.format);
		const GLenum componentCount = VertexFormatTypeComponents(attribute.format);
		const GLuint offset = attribute.byteOffset;

		glEnableVertexArrayAttrib(vertexArrayObject, attribute.locationIndex);
		glVertexArrayAttribBinding(vertexArrayObject, attribute.locationIndex, attribute.bindingIndex);
		glVertexArrayAttribFormat
			vertexArrayObject,
			attribute.locationIndex,
			componentCount,
			vertexFormat,
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
