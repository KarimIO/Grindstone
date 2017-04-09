#include "gl3w.h"
#include "GLUniformBuffer.h"

GRAPHICS_EXPORT UniformBuffer* createUniformBuffer() {
	return new GLUniformBuffer;
}

void GLUniformBuffer::Initialize(int _size) {
	size = _size;

	glGenBuffers(1, &block);
	glBindBuffer(GL_UNIFORM_BUFFER, block);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned int GLUniformBuffer::GetSize() {
	return size;
}

void GLUniformBuffer::Bind() {
	glBindBuffer(GL_UNIFORM_BUFFER, block);
}

void GLUniformBuffer::Setdata(unsigned int offset, unsigned int size, void *value) {
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, value);
}

void GLUniformBuffer::Unbind() {
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}