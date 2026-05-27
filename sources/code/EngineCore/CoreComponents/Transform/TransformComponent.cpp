#include <Common/Math.hpp>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "TransformComponent.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone;

extern "C" {
	ENGINE_CORE_API void* EntityGetTransformComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = Grindstone::EngineCore::GetInstance().GetWorldContextManager()->GetActiveWorldContextSet()->GetEntityRegistry();
		return reg.try_get<TransformComponent>((entt::entity)entity);
	}

	ENGINE_CORE_API Grindstone::Math::ExportableQuaternion TransformComponentGetRotation(const TransformComponent& component) {
		return Grindstone::Math::ExportQuaternion(component.rotation);
	}

	ENGINE_CORE_API void TransformComponentSetRotation(TransformComponent& component, Grindstone::Math::ExportableQuaternion rotation) {
		component.rotation = Grindstone::Math::ImportQuaternion(rotation);
	}

	ENGINE_CORE_API Grindstone::Math::ExportableVector TransformComponentGetPosition(const TransformComponent& component) {
		return Grindstone::Math::ExportVector(component.position);
	}

	ENGINE_CORE_API void TransformComponentSetPosition(TransformComponent& component, Grindstone::Math::ExportableVector position) {
		component.position = Grindstone::Math::ImportVector(position);
	}

	ENGINE_CORE_API Grindstone::Math::ExportableVector TransformComponentGetScale(const TransformComponent& component) {
		return Grindstone::Math::ExportVector(component.scale);
	}

	ENGINE_CORE_API void TransformComponentSetScale(TransformComponent& component, const Grindstone::Math::ExportableVector scale) {
		component.scale = ImportVector(scale);
	}

	ENGINE_CORE_API Grindstone::Math::ExportableVector TransformComponentGetForward(const TransformComponent& component) {
		return Grindstone::Math::ExportVector(component.GetForward());
	}

	ENGINE_CORE_API Grindstone::Math::ExportableVector TransformComponentGetRight(const TransformComponent& component) {
		return Grindstone::Math::ExportVector(component.GetRight());
	}

	ENGINE_CORE_API Grindstone::Math::ExportableVector TransformComponentGetUp(const TransformComponent& component) {
		return Grindstone::Math::ExportVector(component.GetUp());
	}
}

REFLECT_STRUCT_BEGIN(TransformComponent)
	REFLECT_STRUCT_MEMBER(position)
	REFLECT_STRUCT_MEMBER(rotation)
	REFLECT_STRUCT_MEMBER(scale)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
