#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer {
		public:
			struct CreateInfo {
				const char* debugName;
				bool isDynamic;
				uint32_t size;
			};

			virtual void UpdateBuffer(void * content) = 0;
			virtual uint32_t GetSize() = 0;
			virtual void Bind() = 0;
		};
	};
};
