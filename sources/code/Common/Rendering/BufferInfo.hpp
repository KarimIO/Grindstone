#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdint.h>

#include <Common/Graphics/Buffer.hpp>

namespace Grindstone::Renderer {
	struct BufferInfo {
		size_t size = 0;
		Grindstone::GraphicsAPI::BufferUsage usage;
		bool persistent = true;

		bool operator==(const BufferInfo& other) const {
			return size == other.size &&
				usage == other.usage &&
				persistent == other.persistent;
		}

		bool operator!=(const BufferInfo& other) const {
			return !(*this == other);
		}
	};
}
