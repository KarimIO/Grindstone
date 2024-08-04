#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "TransformComponent.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone;

ENGINE_CORE_API typedef struct ExportableVector {
	float x;
	float y;
	float z;
} ExportableVector;

ENGINE_CORE_API typedef struct ExportableQuaternion {
	float x;
	float y;
	float z;
	float w;
} ExportableQuaternion;

glm::vec3 ImportVector(const ExportableVector inVec) {
	return { inVec.x, inVec.y, inVec.z };
}

glm::quat ImportQuaternion(const ExportableQuaternion inQuat) {
	return { inQuat.x, inQuat.y, inQuat.z, inQuat.w };
}

extern "C" {
	ExportableVector ExportVector(const glm::vec3 inVec) {
		return { inVec.x, inVec.y, inVec.z };
	}

	ExportableQuaternion ExportQuaternion(const glm::quat inQuat) {
		return { inQuat.x, inQuat.y, inQuat.z, inQuat.w };
	}

	ENGINE_CORE_API void* EntityGetTransformComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = EngineCore::GetInstance().GetEntityRegistry();
		const entt::entity entityId = static_cast<entt::entity>(entity);
		return &reg.get<TransformComponent>(entityId);
	}

	ENGINE_CORE_API ExportableQuaternion TransformComponentGetRotation(const TransformComponent& component) {
		return ExportQuaternion(component.rotation);
	}

	ENGINE_CORE_API void TransformComponentSetRotation(TransformComponent& component, ExportableQuaternion rotation) {
		component.rotation = ImportQuaternion(rotation);
	}

	ENGINE_CORE_API ExportableVector TransformComponentGetPosition(const TransformComponent& component) {
		return ExportVector(component.position);
	}

	ENGINE_CORE_API void TransformComponentSetPosition(TransformComponent& component, ExportableVector position) {
		component.position = ImportVector(position);
	}

	ENGINE_CORE_API ExportableVector TransformComponentGetScale(const TransformComponent& component) {
		return ExportVector(component.scale);
	}

	ENGINE_CORE_API void TransformComponentSetScale(TransformComponent& component, const ExportableVector scale) {
		component.scale = ImportVector(scale);
	}

	ENGINE_CORE_API ExportableVector TransformComponentGetForward(const TransformComponent& component) {
		return ExportVector(component.GetForward());
	}

	ENGINE_CORE_API ExportableVector TransformComponentGetRight(const TransformComponent& component) {
		return ExportVector(component.GetRight());
	}

	ENGINE_CORE_API ExportableVector TransformComponentGetUp(const TransformComponent& component) {
		return ExportVector(component.GetUp());
	}
}

REFLECT_STRUCT_BEGIN(TransformComponent)
	REFLECT_STRUCT_MEMBER(position)
	REFLECT_STRUCT_MEMBER(rotation)
	REFLECT_STRUCT_MEMBER(scale)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
