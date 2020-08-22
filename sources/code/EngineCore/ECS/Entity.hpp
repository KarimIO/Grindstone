#pragma once

#include <cinttypes>
#include <bitset>

namespace Grindstone {
	namespace ECS {
		using Entity = uint64_t;
		const Entity MAX_ENTITY = UINT64_MAX;
		const Entity INVALID_ENTITY = UINT64_MAX;
	}
}