#pragma once

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Grindstone::GraphicsAPI {
	/*! A Vertex Array Object is a data structure including one or more VertexBuffers
		and an IndexBuffer, to be bound to the GPU simultaneously.
	*/
	class VertexArrayObject {
	public:
		struct CreateInfo {
			const char* debugName = nullptr;
			VertexBuffer** vertexBuffers = nullptr;
			uint32_t vertexBufferCount = 0;
			IndexBuffer* indexBuffer = nullptr;
		};

		virtual ~VertexArrayObject() {};
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};
}
