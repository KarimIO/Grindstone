#include "gl3w.h"
#include "GLVertexArrayObject.h"
#include <iostream>

void GLVertexArrayObject::Initialize()
{
	glGenVertexArrays(1, &vao);

	boundVBOs = 0;
}

void GLVertexArrayObject::BindVBO(VertexBufferObject * vbo)
{
	GLVertexBufferObject *glvbo = (GLVertexBufferObject *)vbo;
	vbos.push_back(vbo);
	boundVBOs = glvbo->Bind(boundVBOs);
}

void GLVertexArrayObject::Bind()
{
	glBindVertexArray(vao);
}

void GLVertexArrayObject::Unbind()
{
	glBindVertexArray(0);
}

void GLVertexArrayObject::Cleanup()
{
	glDeleteVertexArrays(1, &vao);
}

void GLVertexArrayObject::CleanupVBOs()
{
	for (size_t i = 0; i < boundVBOs; i++)
		vbos[i]->Cleanup();

	glDeleteVertexArrays(1, &vao);
}

GRAPHICS_EXPORT VertexArrayObject* createVAO() {
	return new GLVertexArrayObject;
}