#ifndef _GL_VERTEX_BUFFER_H
#define _GL_VERTEX_BUFFER_H

#include <stdint.h>
#include <Common/Graphics/VertexBuffer.hpp>


namespace Grindstone {
	namespace GraphicsAPI {
		class GLVertexBuffer : public VertexBuffer {
		private:
			uint32_t size_;
			GLuint buffer_;
			VertexBufferLayout layout_;
		public:
			GLVertexBuffer(CreateInfo& createInfo);
			~GLVertexBuffer();

			VertexBufferLayout &getLayout();

			virtual void updateBuffer(void *content);

			void bind();
		};
	}
}

#endif