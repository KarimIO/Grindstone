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
			GLuint uniformBufferObject;
			GLuint bindingLocation;
			uint32_t size;
		public:
			GLUniformBuffer(CreateInfo& createInfo);
			~GLUniformBuffer();

			virtual void Bind() override;
			virtual void UpdateBuffer(void *content) override;
		};
	}
}

#endif