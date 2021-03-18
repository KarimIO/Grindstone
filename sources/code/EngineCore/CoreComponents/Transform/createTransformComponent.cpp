#include <entt/entt.hpp>

#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "createTransformComponent.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "TransformComponent.hpp"
using namespace Grindstone;

void* Grindstone::createTransformComponent(entt::registry& registry, ECS::Entity entity) {
	return &registry.emplace<TransformComponent>(entity);
}

REFLECT_STRUCT_BEGIN(TransformComponent)
	// REFLECT_STRUCT_MEMBER_D(position, "Position", "position", Reflection::Metadata::SaveSetAndView, nullptr)
	// REFLECT_STRUCT_MEMBER_D(angles, "Rotation", "rotation", Reflection::Metadata::SaveSetAndView, nullptr)
	// REFLECT_STRUCT_MEMBER_D(scale, "Scale", "scale", Reflection::Metadata::SaveSetAndView, nullptr)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
