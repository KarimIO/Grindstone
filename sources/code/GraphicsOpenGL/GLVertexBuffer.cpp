#include <GL/gl3w.h>
#include "GLVertexBuffer.hpp"
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		GLVertexBuffer::GLVertexBuffer(VertexBufferCreateInfo ci) : size_(ci.size) {
			layout_ = *ci.layout;
			glGenBuffers(1, &buffer_);
			glBindBuffer(GL_ARRAY_BUFFER, buffer_);
			glBufferData(GL_ARRAY_BUFFER, ci.size, ci.content, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		VertexBufferLayout &GLVertexBuffer::getLayout() {
			return layout_;
		}

		void GLVertexBuffer::bind() {
			glBindBuffer(GL_ARRAY_BUFFER, buffer_);
		}

		void GLVertexBuffer::updateBuffer(void *content) {
			glBindBuffer(GL_ARRAY_BUFFER, buffer_);
			void* p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			memcpy(p, content, size_);
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}

		GLVertexBuffer::~GLVertexBuffer() {
			glDeleteBuffers(1, &buffer_);
		}
	}
}
