#pragma once

#include <cinttypes>

namespace Grindstone {
	namespace ECS {
		using ComponentType = std::uint8_t;
		const ComponentType MAX_COMPONENTS = UINT8_MAX;
	}
}
