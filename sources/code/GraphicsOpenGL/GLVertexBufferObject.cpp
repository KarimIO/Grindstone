#include "gl3w.h"
#include "GLVertexBufferObject.h"
#include <iostream>

unsigned int *GLVertexBufferObject::GetHandle()
{
	return vboHandle;
}

uint8_t GLVertexBufferObject::Bind(uint8_t bindTo)
{
	uint8_t i;
	for (i = 0; i < size; i++) {
		Bind(bindTo + i, i, false, 0u, 0u);
	}
	return bindTo + i;
}

void GLVertexBufferObject::Bind(uint8_t bindTo, uint8_t id, bool normalize, uint32_t stride, uint32_t offset)
{
	GLuint err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Bind VBO Start: " << err << std::endl;
	}
	GLvoid const* pointer = static_cast<char const*>(0) + offset;
	glEnableVertexAttribArray(bindTo);
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[id]);
	glVertexAttribPointer(
		bindTo,	
		elementSize,
		GL_BYTE + dataSizeType,           // type
		normalize,           // normalized?
		stride,                  // stride
		pointer            // array buffer offset
	);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Bind VBO End: " << err << std::endl;
	}
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
	numBuffers = 0;
}

void GLVertexBufferObject::AddVBO(void * data, uint64_t bufferSize, uint8_t elementSize, dataSize dataSz, drawType drawTyp)
{
	GLuint err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error ADD VBO Start: " << err << std::endl;
	}
	int drawGL = GL_STATIC_DRAW;
	if (drawTyp == DRAW_STREAM)
		drawGL = GL_STREAM_DRAW;
	else if (drawTyp == DRAW_DYNAMIC)
		drawGL = GL_DYNAMIC_DRAW;

	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[numBuffers++]);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, data, drawGL);

	this->elementSize = elementSize;
	dataSizeType = dataSz;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Add VBO end: " << err << std::endl;
	}
}

void GLVertexBufferObject::AddIBO(void *data, uint64_t bufferSize, drawType drawTyp) {
	GLuint err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Bind IBO Start: " << err << std::endl;
	}
	int drawGL = GL_STATIC_DRAW;
	if (drawTyp == DRAW_STREAM)
		drawGL = GL_STREAM_DRAW;
	else if (drawTyp == DRAW_DYNAMIC)
		drawGL = GL_DYNAMIC_DRAW;

	this->elementSize = elementSize;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Bind IBO Mid 1: " << err << std::endl;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandle[numBuffers++]);
	this->elementSize = elementSize;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Bind IBO Mid 2: " << err << std::endl;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, data, drawGL);

	this->elementSize = elementSize;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error Bind IBO End: " << err << std::endl;
	}
}

void GLVertexBufferObject::AddVBO(VertexBufferObjectInitializer vbo)
{
	AddVBO(vbo.data, vbo.size, vbo.strideSize, vbo.dataSz, vbo.drawTp);
}

void GLVertexBufferObject::AddVBO(std::vector<VertexBufferObjectInitializer> vbos)
{
	for (size_t i = 0; i < vbos.size(); i++) {
		AddVBO(vbos[i].data, vbos[i].size, vbos[i].strideSize, vbos[i].dataSz, vbos[i].drawTp);
	}
}

GRAPHICS_EXPORT VertexBufferObject* createVBO() {
	return new GLVertexBufferObject;
}