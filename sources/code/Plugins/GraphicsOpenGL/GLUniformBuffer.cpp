#include <GL/gl3w.h>
#include "GLUniformBuffer.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		GLUniformBuffer::GLUniformBuffer(CreateInfo& ci) : size(ci.size) {
			glGenBuffers(1, &uniformBufferObject);
			glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, ci.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			GLUniformBufferBinding *ubb = (GLUniformBufferBinding *)ci.binding;
			bindingLocation = ubb->GetBindingLocation();
			glBindBufferBase(GL_UNIFORM_BUFFER, bindingLocation, uniformBufferObject);
		}

		void GLUniformBuffer::Bind() {
			glBindBufferBase(GL_UNIFORM_BUFFER, bindingLocation, uniformBufferObject);
		}

		GLUniformBuffer::~GLUniformBuffer() {
			glDeleteBuffers(1, &uniformBufferObject);
		}

		void GLUniformBuffer::UpdateBuffer(void * content) {
			glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject);
			GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
			memcpy(p, content, size);
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		GLUniformBufferBinding::GLUniformBufferBinding(CreateInfo& createInfo) {
			bindingLocation = createInfo.binding;
			uniformName = createInfo.shaderLocation;
		}

		const char * GLUniformBufferBinding::GetUniformName() {
			return uniformName;
		}

		GLuint GLUniformBufferBinding::GetBindingLocation() {
			return bindingLocation;
		}
	}
}