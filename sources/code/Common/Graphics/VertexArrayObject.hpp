#ifndef _VERTEX_ARRAY_OBJECT_H
#define _VERTEX_ARRAY_OBJECT_H

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {

		class VertexArrayObject {
		public:
			struct CreateInfo {
				VertexBuffer** vertex_buffers;
				uint32_t vertex_buffer_count;
				IndexBuffer* index_buffer;
			};

			virtual ~VertexArrayObject() {};
			virtual void bind() = 0;
			virtual void unbind() = 0;
		};
	};
};

#endif