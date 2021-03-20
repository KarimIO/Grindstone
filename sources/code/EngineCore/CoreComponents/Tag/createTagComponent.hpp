#pragma once

#include <entt/entt.hpp>
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	void* createTagComponent(entt::registry& registry, ECS::Entity entity);
	bool tryGetEntityTagComponent(entt::registry& registry, ECS::Entity entity, void* outEntity);
}
