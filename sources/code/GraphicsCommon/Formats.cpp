#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		ColorMask operator~(const ColorMask& f) {
			return ColorMask(~static_cast<int>(f));
		}

		ColorMask operator|(const ColorMask& a, const ColorMask& b) {
			return ColorMask(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}

		ColorMask operator&(const ColorMask& a, const ColorMask& b) {
			return ColorMask(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}
	}
}
