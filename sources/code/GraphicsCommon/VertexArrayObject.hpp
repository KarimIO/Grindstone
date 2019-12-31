#ifndef _VERTEX_ARRAY_OBJECT_H
#define _VERTEX_ARRAY_OBJECT_H

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct VertexArrayObjectCreateInfo {
			VertexBuffer *vertexBuffer;
			IndexBuffer *indexBuffer;
		};

		class VertexArrayObject {
		public:
			virtual ~VertexArrayObject() {};
			virtual void Bind() {};
			virtual void BindResources(VertexArrayObjectCreateInfo createInfo) {};
			virtual void Unbind() {};
		};
	};
};

#endif