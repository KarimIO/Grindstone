#include <cstring>

#include <GL/gl3w.h>

#include "GLBuffer.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::Buffer::Buffer(const Grindstone::GraphicsAPI::Buffer::CreateInfo& createInfo) : GraphicsAPI::Buffer(createInfo) {
	if (createInfo.bufferUsage.Test(BufferUsage::Vertex)) {
		bufferType = GL_ARRAY_BUFFER;
	}
	else if (createInfo.bufferUsage.Test(BufferUsage::Index)) {
		bufferType = GL_ELEMENT_ARRAY_BUFFER;
	}
	else if (createInfo.bufferUsage.Test(BufferUsage::Uniform)) {
		bufferType = GL_UNIFORM_BUFFER;
	}
	else if (createInfo.bufferUsage.Test(BufferUsage::Storage)) {
		bufferType = GL_SHADER_STORAGE_BUFFER;
	}
	else if (createInfo.bufferUsage.Test(BufferUsage::Indirect)) {
		bufferType = GL_DRAW_INDIRECT_BUFFER;
	}
	else {
		bufferType = GL_ARRAY_BUFFER;
	}

	GLenum usageHint = 0;
	switch (createInfo.memoryUsage) {
	case MemUsage::GPUOnly:		usageHint = GL_STATIC_DRAW; break;
	case MemUsage::CPUOnly:		usageHint = GL_STREAM_DRAW; break;
	case MemUsage::CPUToGPU:	usageHint = GL_DYNAMIC_DRAW; break;
	case MemUsage::GPUToCPU:	usageHint = GL_DYNAMIC_READ; break;
	}

	glGenBuffers(1, &bufferObject);
	glBindBuffer(bufferType, bufferObject);
	glObjectLabel(GL_BUFFER, bufferObject, -1, createInfo.debugName);
	glBufferData(bufferType, createInfo.bufferSize, createInfo.content, usageHint);
	glBindBuffer(bufferType, 0);
}

OpenGL::Buffer::~Buffer() {
	glDeleteBuffers(1, &bufferObject);
}

void* OpenGL::Buffer::Map() {
	glBindBuffer(bufferType, bufferObject);
	return glMapBuffer(bufferType, GL_WRITE_ONLY);
}

void OpenGL::Buffer::Unmap() {
	glBindBuffer(bufferType, bufferObject);
	glUnmapBuffer(bufferType);
}

void OpenGL::Buffer::UploadData(const void* data, size_t size, size_t offset) {
	glBindBuffer(bufferType, bufferObject);
	glBufferSubData(bufferType, offset, size, data);
}

GLuint Grindstone::GraphicsAPI::OpenGL::Buffer::GetBuffer() const {
	return bufferObject;
}
