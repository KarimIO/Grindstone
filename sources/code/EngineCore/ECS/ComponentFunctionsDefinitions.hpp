#pragma once

#include "ComponentFunctions.hpp"

namespace Grindstone::ECS {
	template<typename ComponentType>
	void* CreateComponent(ECS::Entity& entity) {
		return &entity.GetSceneEntityRegistry().emplace<ComponentType>(entity.GetHandle());
	}

	template<typename ComponentType>
	void RemoveComponent(ECS::Entity& entity) {
		auto& registry = entity.GetSceneEntityRegistry();
		auto entityHandle = entity.GetHandle();
		if (registry.any_of<ComponentType>(entityHandle)) {
			registry.remove<ComponentType>(entityHandle);
		}
	}

	template<typename ComponentType>
	Grindstone::Reflection::TypeDescriptor_Struct GetComponentReflectionData() {
		return ComponentType::reflectionInfo;
	}

	template<typename ComponentType>
	bool TryGetComponent(ECS::Entity& entity, void*& outComponent) {
		auto& registry = entity.GetSceneEntityRegistry();
		auto entityHandle = entity.GetHandle();
		if (registry.any_of<ComponentType>(entityHandle)) {
			outComponent = &registry.get<ComponentType>(entityHandle);
			return true;
		}

		outComponent = nullptr;
		return false;
	}

	template<typename ComponentType>
	bool HasComponent(ECS::Entity& entity) {
		auto& registry = entity.GetSceneEntityRegistry();
		auto entityHandle = entity.GetHandle();
		return registry.all_of<ComponentType>(entityHandle);
	}

	template<typename ComponentType>
	void CopyRegistryComponents(entt::registry& dst, entt::registry& src) {
		auto& srcEntities = src.view<ComponentType>();

		for (entt::entity srcEntity : srcEntities) {
			ComponentType srcComponent = src.get<ComponentType>(srcEntity);
			dst.emplace_or_replace<ComponentType>(srcEntity, srcComponent);
		}
	}
}
