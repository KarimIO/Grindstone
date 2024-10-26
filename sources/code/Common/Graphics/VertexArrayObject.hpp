#pragma once

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Grindstone::GraphicsAPI {
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
