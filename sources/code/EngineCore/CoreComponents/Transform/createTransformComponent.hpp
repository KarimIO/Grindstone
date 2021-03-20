#pragma once

#include <entt/entt.hpp>
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	void* createTransformComponent(entt::registry& registry, ECS::Entity entity);
}
