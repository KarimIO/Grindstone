#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace Formats {
		namespace Model {
			enum class IndexSize : uint8_t {
				Bit16,
				Bit32
			};

			namespace Header {
				struct V1 {
					uint32_t totalFileSize = 0;
					uint32_t version = 0;
					uint32_t meshCount = 0;
					uint64_t vertexCount = 0;
					uint64_t indexCount = 0;
					IndexSize isUsing32BitIndices = IndexSize::Bit16;
				};
			}
		}
	}
}