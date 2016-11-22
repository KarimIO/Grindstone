#include "GLVertexArrayObject.h"
#include "gl3w.h"

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
	glDeleteVertexArrays(1, &vao);
}
