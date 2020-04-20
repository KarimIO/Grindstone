#ifndef _VERTEX_ARRAY_OBJECT_H
#define _VERTEX_ARRAY_OBJECT_H

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct VertexArrayObjectCreateInfo {
			VertexBuffer** vertex_buffers;
			uint32_t vertex_buffer_count;
			IndexBuffer* index_buffer;
		};

		class VertexArrayObject {
		public:
			virtual ~VertexArrayObject() {};
			virtual void Bind() = 0;
			virtual void Unbind() = 0;
		};
	};
};

#endif