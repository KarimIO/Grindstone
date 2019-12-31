#include <GL/gl3w.h>
#include "GLUniformBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		GLUniformBuffer::GLUniformBuffer(UniformBufferCreateInfo ci) {
			size = ci.size;

			glGenBuffers(1, &ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, ci.isDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			GLUniformBufferBinding *ubb = (GLUniformBufferBinding *)ci.binding;
			bindingLocation = ubb->GetBindingLocation();
			glBindBufferBase(GL_UNIFORM_BUFFER, bindingLocation, ubo);
		}

		void GLUniformBuffer::Bind() {
			glBindBufferBase(GL_UNIFORM_BUFFER, bindingLocation, ubo);
		}

		GLUniformBuffer::~GLUniformBuffer() {
			glDeleteBuffers(1, &ubo);
		}

		void GLUniformBuffer::UpdateUniformBuffer(void * content) {
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferData(GL_UNIFORM_BUFFER, size, content, GL_DYNAMIC_DRAW);
			//GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
			//memcpy(p, content, size);
			//glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		GLUniformBufferBinding::GLUniformBufferBinding(UniformBufferBindingCreateInfo createInfo) {
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