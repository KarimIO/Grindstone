#pragma once

#include <stdint.h>

namespace Grindstone {
	struct Display {
	public:
		// colorBuffer	Color RenderBuffer.
		// depthBuffer	Depth RenderBuffer.
		uint32_t monitor_id_;
		uint32_t x_, y_;
		uint32_t width_, height_;
	};
}
