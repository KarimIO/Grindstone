#pragma once

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/DLLDefs.hpp>
#include "Display.hpp"

namespace Grindstone {
	class DisplayManager {
	public:
		virtual Grindstone::Display getMainDisplay();
		virtual uint8_t getDisplayCount();
		virtual void enumerateDisplays(Grindstone::Display*);
	};
}
