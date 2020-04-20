#ifndef _GL_UNIFORM_BUFFER_H
#define _GL_UNIFORM_BUFFER_H

#include "../GraphicsCommon/UniformBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GLUniformBufferBinding : public UniformBufferBinding {
			const char *uniformName;
			GLuint bindingLocation;
		public:
			GLUniformBufferBinding(UniformBufferBindingCreateInfo);
			const char *GetUniformName();
			GLuint GetBindingLocation();
		};

		class GLUniformBuffer : public UniformBuffer {
		private:
			GLuint ubo_;
			GLuint binding_location_;
			uint32_t size_;
		public:
			GLUniformBuffer(UniformBufferCreateInfo ci);
			void Bind();
			~GLUniformBuffer();

			void updateBuffer(void * content);
		};
	}
}

#endif