#include <GL/gl3w.h>
#include <iostream>

#include "GLIndexBuffer.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::IndexBuffer::IndexBuffer(const IndexBuffer::CreateInfo& createInfo) {
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	if (createInfo.debugName != nullptr) {
		glObjectLabel(GL_BUFFER, buffer, -1, createInfo.debugName);
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, createInfo.size, createInfo.content, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

OpenGL::IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &buffer);
}

void OpenGL::IndexBuffer::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
}
