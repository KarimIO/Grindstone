#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace GraphicsAPI {

		class IndexBuffer {
		public:
			struct CreateInfo {
				const void* content;
				uint32_t size;
				uint32_t count;
			};

			virtual ~IndexBuffer() {};
		};
	}
}