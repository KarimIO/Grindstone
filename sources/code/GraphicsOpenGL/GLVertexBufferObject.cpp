#include "GLVertexBufferObject.h"

unsigned int *GLVertexBufferObject::GetHandle()
{
	return vboHandle;
}

uint8_t GLVertexBufferObject::Bind(uint8_t bindTo)
{
	uint8_t i;
	for (i = 0; i < size; i++) {
		Bind(bindTo + i, i);
	}
	return bindTo + i;
}

void GLVertexBufferObject::Bind(uint8_t bindTo, uint8_t id)
{
	glEnableVertexAttribArray(bindTo);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);
	glVertexAttribPointer(
		bindTo,	
		3,
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
}

void GLVertexBufferObject::Unbind()
{
	for (uint8_t i = 0; i < size; i++) {
		Unbind(i);
	}
}

void GLVertexBufferObject::Unbind(uint8_t id)
{
	glDisableVertexAttribArray(id);
}

void GLVertexBufferObject::Cleanup()
{
	glDeleteBuffers(size, vboHandle);
}

void GLVertexBufferObject::Initialize(uint8_t size)
{
	vboHandle = new unsigned int[size];
	glGenBuffers(size, vboHandle);
	this->size = size;
}

void GLVertexBufferObject::AddVBO(void * data, uint64_t size, uint8_t strideSize, uint8_t dataSize)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void GLVertexBufferObject::AddVBO(VertexBufferObjectInitializer vbo)
{
	AddVBO(vbo.data, vbo.size, vbo.strideSize, vbo.dataSize);
}

void GLVertexBufferObject::AddVBO(std::vector<VertexBufferObjectInitializer> vbos)
{
	for (size_t i = 0; i < vbos.size(); i++) {
		AddVBO(vbos[i].data, vbos[i].size, vbos[i].strideSize, vbos[i].dataSize);
	}
}
