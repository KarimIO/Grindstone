#include <GL/gl3w.h>
#include "GLVertexBuffer.hpp"
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		GLVertexBuffer::GLVertexBuffer(CreateInfo& ci) : size(ci.size), vertexLayout(*ci.layout){
			glGenBuffers(1, &vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, ci.size, ci.content, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		VertexBufferLayout &GLVertexBuffer::GetLayout() {
			return vertexLayout;
		}

		void GLVertexBuffer::Bind() {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		}

		void GLVertexBuffer::UpdateBuffer(void *content) {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			void* p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			memcpy(p, content, size);
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}

		GLVertexBuffer::~GLVertexBuffer() {
			glDeleteBuffers(1, &vertexBuffer);
		}
	}
}
