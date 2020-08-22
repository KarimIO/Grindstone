#pragma once

#include <vector>
#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/DLLDefs.hpp>

namespace Grindstone {
	struct Display {
	public:
		// colorBuffer	Color RenderBuffer.
		// depthBuffer	Depth RenderBuffer.
		uint32_t monitor_id_;
		uint32_t x_, y_;
		uint32_t width_, height_;
	};

	namespace Displays {
		Grindstone::Display getMainDisplay();
		uint8_t getDisplayCount();
		void    enumerateDisplays(Grindstone::Display*);
	}
};