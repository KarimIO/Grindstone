#pragma once

#include "EngineCore/Reflection/ComponentReflection.hpp"

#include <entt/fwd.hpp>

namespace Grindstone {
	struct ParentComponent {
		entt::entity parentEntity = entt::null;

		REFLECT("Parent")
	};
}
