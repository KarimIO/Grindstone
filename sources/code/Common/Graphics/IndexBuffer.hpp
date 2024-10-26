#pragma once

#include <stdint.h>

namespace Grindstone::GraphicsAPI {
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
