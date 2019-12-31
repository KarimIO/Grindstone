#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace GraphicsAPI {
		struct IndexBufferCreateInfo {
			const void *content;
			uint32_t size;
			uint32_t count;
		};

		class IndexBuffer {
		public:
			virtual ~IndexBuffer() {};
		};
	}
}