#ifndef _VERTEX_ARRAY_OBJECT_H
#define _VERTEX_ARRAY_OBJECT_H

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {

		class VertexArrayObject {
		public:
			struct CreateInfo {
				VertexBuffer** vertexBuffers;
				uint32_t vertexBufferCount;
				IndexBuffer* indexBuffer;
			};

			virtual ~VertexArrayObject() {};
			virtual void Bind() = 0;
			virtual void Unbind() = 0;
		};
	};
};

#endif