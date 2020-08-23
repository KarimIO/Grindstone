#include <GL/gl3w.h>
#include "GLUniformBuffer.hpp"
#include <cstring>

namespace Grindstone {
	namespace GraphicsAPI {
		GLUniformBuffer::GLUniformBuffer(CreateInfo& ci) : size_(ci.size) {
			glGenBuffers(1, &ubo_);
			glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
			glBufferData(GL_UNIFORM_BUFFER, size_, nullptr, ci.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			GLUniformBufferBinding *ubb = (GLUniformBufferBinding *)ci.binding;
			binding_location_ = ubb->GetBindingLocation();
			glBindBufferBase(GL_UNIFORM_BUFFER, binding_location_, ubo_);
		}

		void GLUniformBuffer::bind() {
			glBindBufferBase(GL_UNIFORM_BUFFER, binding_location_, ubo_);
		}

		GLUniformBuffer::~GLUniformBuffer() {
			glDeleteBuffers(1, &ubo_);
		}

		void GLUniformBuffer::updateBuffer(void * content) {
			glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
			GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
			memcpy(p, content, size_);
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