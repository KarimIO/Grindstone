#include <GL/gl3w.h>
#include "GLIndexBuffer.hpp"
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
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
	}
}