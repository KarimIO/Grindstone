#pragma once

#include <stdint.h>

namespace Grindstone::Renderer {
	enum class GpuPassType : uint8_t {
		None,
		Graphics,
		Compute,
		Transfer,
		Present
	};
}
