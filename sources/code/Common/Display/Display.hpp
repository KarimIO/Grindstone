#pragma once

#include <stdint.h>

namespace Grindstone {
	struct Display {
	public:
		// colorBuffer	Color RenderBuffer.
		// depthBuffer	Depth RenderBuffer.
		uint32_t monitorId = 0;
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t width = 0;
		uint32_t height = 0;
	};
}
