#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;

	namespace ECS {
		using SystemFactory = void(*)(entt::registry&);
	}
}
