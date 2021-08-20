#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace GraphicsAPI {

		class IndexBuffer {
		public:
			struct CreateInfo {
				const char* debugName = nullptr;
				const void* content = nullptr;
				uint32_t size = 0;
				uint32_t count = 0;
			};

			virtual ~IndexBuffer() {};
		};
	}
}