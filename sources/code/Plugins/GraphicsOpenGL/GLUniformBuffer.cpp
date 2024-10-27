#include <cstring>

#include <GL/gl3w.h>

#include "GLUniformBuffer.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::UniformBuffer::UniformBuffer(const UniformBuffer::CreateInfo& ci) : size(ci.size) {
	glGenBuffers(1, &uniformBufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, ci.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	glObjectLabel(GL_BUFFER, uniformBufferObject, -1, ci.debugName);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBufferObject);
}

OpenGL::UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &uniformBufferObject);
}

void OpenGL::UniformBuffer::UpdateBuffer(void* content) {
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, content, size);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

uint32_t OpenGL::UniformBuffer::GetSize() const {
	return size;
}

void OpenGL::UniformBuffer::Bind() {
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingLocation, uniformBufferObject);
}
