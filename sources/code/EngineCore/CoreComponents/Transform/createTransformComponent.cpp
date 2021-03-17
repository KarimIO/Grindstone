#include <entt/entt.hpp>

#include "EngineCore/ECS/Entity.hpp"
#include "createTransformComponent.hpp"
#include "TransformComponent.hpp"
using namespace Grindstone;

void* Grindstone::createTransformComponent(entt::registry& registry, ECS::Entity entity) {
	return &registry.emplace<TransformComponent>(entity);
}
