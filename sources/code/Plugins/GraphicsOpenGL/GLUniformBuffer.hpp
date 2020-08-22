#ifndef _GL_UNIFORM_BUFFER_H
#define _GL_UNIFORM_BUFFER_H

#include <Common/Graphics/UniformBuffer.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLUniformBufferBinding : public UniformBufferBinding {
			const char *uniformName;
			GLuint bindingLocation;
		public:
			GLUniformBufferBinding(CreateInfo& ci);
			const char *GetUniformName();
			GLuint GetBindingLocation();
		};

		class GLUniformBuffer : public UniformBuffer {
		private:
			GLuint ubo_;
			GLuint binding_location_;
			uint32_t size_;
		public:
			GLUniformBuffer(CreateInfo& ci);
			virtual void bind() override;
			~GLUniformBuffer();

			virtual void updateBuffer(void * content) override;
		};
	}
}

#endif