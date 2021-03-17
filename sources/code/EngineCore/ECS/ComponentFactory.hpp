#pragma once

#include <entt/entt.hpp>

#include "EngineCore/ECS/Entity.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		using ComponentFactory = void*(*)(entt::registry& registry, ECS::Entity entity);
	}
}
