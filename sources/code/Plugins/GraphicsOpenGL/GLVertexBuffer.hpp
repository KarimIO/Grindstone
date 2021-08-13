#ifndef _GL_VERTEX_BUFFER_H
#define _GL_VERTEX_BUFFER_H

#include <stdint.h>
#include <Common/Graphics/VertexBuffer.hpp>


namespace Grindstone {
	namespace GraphicsAPI {
		class GLVertexBuffer : public VertexBuffer {
		private:
			uint32_t size;
			GLuint vertexBuffer;
			VertexBufferLayout vertexLayout;
		public:
			GLVertexBuffer(CreateInfo& createInfo);
			~GLVertexBuffer();

			VertexBufferLayout &GetLayout();
			virtual void UpdateBuffer(void *content);
			void Bind();
		};
	}
}

#endif