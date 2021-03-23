#pragma once

#include <stdint.h>

namespace Grindstone {
	struct Display {
	public:
		// colorBuffer	Color RenderBuffer.
		// depthBuffer	Depth RenderBuffer.
		uint32_t monitorId;
		uint32_t x, y;
		uint32_t width, height;
	};
}
