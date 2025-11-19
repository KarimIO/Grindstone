#pragma once

#include <stdint.h>

namespace Grindstone::Renderer {
	enum class GpuQueue : uint8_t {
		None,
		Graphics,
		Compute
	};
}
