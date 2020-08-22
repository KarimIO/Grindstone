#ifndef _GL_INDEX_BUFFER_H
#define _GL_INDEX_BUFFER_H

#include <stdint.h>
#include <Common/Graphics/DLLDefs.hpp>
#include <Common/Graphics/IndexBuffer.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLIndexBuffer : public IndexBuffer {
			GLuint buffer;
		public:
			GLIndexBuffer(CreateInfo& createInfo);
			~GLIndexBuffer();

			void Bind();
		};
	}
}

#endif