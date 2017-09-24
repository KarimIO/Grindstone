#include <GL/gl3w.h>
#include "GLVertexArrayObject.h"
#include <iostream>

GLVertexArrayObject::GLVertexArrayObject(VertexArrayObjectCreateInfo createInfo) {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

GLVertexArrayObject::~GLVertexArrayObject() {
	glDeleteVertexArrays(1, &vao);
}

void GLVertexArrayObject::Bind() {
	glBindVertexArray(vao);
}

void GLVertexArrayObject::BindResources(VertexArrayObjectCreateInfo createInfo) {
}

void GLVertexArrayObject::Unbind() {
	glBindVertexArray(0);
}
