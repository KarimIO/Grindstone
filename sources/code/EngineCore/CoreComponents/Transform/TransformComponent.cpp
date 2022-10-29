#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "TransformComponent.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API void* EntityGetTransformComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = scene->GetEntityRegistry();
		return &reg.get<TransformComponent>((entt::entity)entity);
	}

	ENGINE_CORE_API void TransformComponentGetRotation(TransformComponent& component, Math::Quaternion* outQuat) {
		*outQuat = component.rotation;
	}

	ENGINE_CORE_API void TransformComponentSetRotation(TransformComponent& component, Math::Quaternion rotation) {
		component.rotation = rotation;
	}

	ENGINE_CORE_API void TransformComponentGetPosition(TransformComponent& component, glm::vec3* outPos) {
		*outPos = component.position;
	}

	ENGINE_CORE_API void TransformComponentSetPosition(TransformComponent& component, Math::Float3 position) {
		component.position = position;
	}

	ENGINE_CORE_API void TransformComponentGetScale(TransformComponent& component, glm::vec3* outScale) {
		*outScale = component.scale;
	}

	ENGINE_CORE_API void TransformComponentSetScale(TransformComponent& component, Math::Float3 scale) {
		component.scale = scale;
	}
}

REFLECT_STRUCT_BEGIN(TransformComponent)
	REFLECT_STRUCT_MEMBER(position)
	REFLECT_STRUCT_MEMBER(rotation)
	REFLECT_STRUCT_MEMBER(scale)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
