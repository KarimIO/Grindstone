#ifndef _GL_UNIFORM_BUFFER_H
#define _GL_UNIFORM_BUFFER_H

#include <Common/Graphics/UniformBuffer.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLUniformBuffer : public UniformBuffer {
		public:
			GLUniformBuffer(CreateInfo& createInfo);
			~GLUniformBuffer();

			// Inherited via UniformBuffer
			virtual void UpdateBuffer(void *content) override;
			virtual uint32_t GetSize() override;
			virtual void Bind() override;
		private:
			GLuint uniformBufferObject;
			GLuint bindingLocation;
			uint32_t size;
		};
	}
}

#endif
