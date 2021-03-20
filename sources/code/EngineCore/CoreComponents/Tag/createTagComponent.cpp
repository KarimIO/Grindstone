#include <entt/entt.hpp>

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "createTagComponent.hpp"
#include "TagComponent.hpp"
using namespace Grindstone;

void* Grindstone::createTagComponent(entt::registry& registry, ECS::Entity entity) {
	return &registry.emplace<TagComponent>(entity);
}

REFLECT_STRUCT_BEGIN(TagComponent)
	REFLECT_STRUCT_MEMBER(tag)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
