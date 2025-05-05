#pragma once

#include "Buffer.hpp"

namespace Grindstone::GraphicsAPI {
	/*! A Vertex Array Object is a data structure including one or more VertexBuffers
		and an IndexBuffer, to be bound to the GPU simultaneously.
	*/
	class VertexArrayObject {
	public:
		struct CreateInfo {
			const char* debugName = nullptr;
			Buffer** vertexBuffers = nullptr;
			uint32_t vertexBufferCount = 0;
			Buffer* indexBuffer = nullptr;
			VertexInputLayout layout;
		};

		virtual ~VertexArrayObject() {};
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};
}
