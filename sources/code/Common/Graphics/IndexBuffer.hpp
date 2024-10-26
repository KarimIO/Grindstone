#pragma once

#include <stdint.h>

namespace Grindstone::GraphicsAPI {
	/*! An IndexBuffer is a list of numbers, to be used with a VertexBuffer as indices
		to avoid redundancies in data. If two triangles share the same vertex data
		including position, normals, and other vertex data, they can be represented by
		a single vertex, often dramatically reducing the memory costs of a mesh.
	*/
	class IndexBuffer {
	public:
		struct CreateInfo {
			const char* debugName = nullptr;
			const void* content = nullptr;
			uint32_t size = 0;
			uint32_t count = 0;
			bool is32Bit = false;
		};

		virtual ~IndexBuffer() {};
	};
}
