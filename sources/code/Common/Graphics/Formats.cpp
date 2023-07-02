#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		ShaderStageBit operator |(ShaderStageBit a, ShaderStageBit b) {
			return static_cast<ShaderStageBit>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}

		ColorMask operator~(const ColorMask& f) {
			return static_cast<ColorMask>(~static_cast<int>(f));
		}

		ColorMask operator|(const ColorMask& a, const ColorMask& b) {
			return static_cast<ColorMask>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}

		ColorMask operator&(const ColorMask& a, const ColorMask& b) {
			return static_cast<ColorMask>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}
	}
}
