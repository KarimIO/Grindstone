#include <GL/gl3w.h>
#include "GLIndexBuffer.hpp"
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		GLIndexBuffer::GLIndexBuffer(CreateInfo& createInfo) {
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
			if (createInfo.debugName != nullptr) {
				glObjectLabel(GL_BUFFER, buffer, -1, createInfo.debugName);
			}
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, createInfo.size, createInfo.content, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		GLIndexBuffer::~GLIndexBuffer() {
			glDeleteBuffers(1, &buffer);
		}

		void GLIndexBuffer::Bind() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		}
	}
}