#include <GL/gl3w.h>
#include "GLIndexBuffer.h"
#include <iostream>

GLIndexBuffer::GLIndexBuffer(IndexBufferCreateInfo createInfo) {
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, createInfo.size, createInfo.content, GL_STATIC_DRAW);
}

GLIndexBuffer::~GLIndexBuffer() {
	glDeleteBuffers(1, &buffer);
}

void GLIndexBuffer::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
}
