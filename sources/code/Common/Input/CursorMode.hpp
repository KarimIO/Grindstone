#pragma once

#include <stdint.h>

namespace Grindstone {
	namespace Input {
		enum class CursorMode : uint8_t {
			Normal,		// Normal use case for cursor
			Hidden,		// Cursor not visible
			Disabled	// Locks cursor, hides it, but captures input deltas
		};
	} // namespace Input
} // namespace Grindstone
