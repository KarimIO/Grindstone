#pragma once

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/DLLDefs.hpp>
#include "Display.hpp"

namespace Grindstone {
	class DisplayManager {
	public:
		virtual Grindstone::Display GetMainDisplay() const;
		virtual uint8_t GetDisplayCount() const;
		virtual void EnumerateDisplays(Grindstone::Display*) const;
	};
}
