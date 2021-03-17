#include <entt/entt.hpp>

#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"

namespace Grindstone {
	void* createTransformComponent(entt::registry& registry, ECS::Entity entity);
}
