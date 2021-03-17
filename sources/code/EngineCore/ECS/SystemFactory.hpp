#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace ECS {
		using SystemFactory = void(*)(entt::registry&);
	}
}
