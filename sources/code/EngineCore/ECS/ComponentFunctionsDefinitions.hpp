#pragma once

#include "ComponentFunctions.hpp"

namespace Grindstone::ECS {
	template<typename ComponentType>
	void* CreateComponent(entt::registry& registry, entt::entity entity) {
		return &registry.emplace<ComponentType>(entity);
	}

	template<typename ComponentType>
	void RemoveComponent(entt::registry& registry, entt::entity entity) {
		if (registry.any_of<ComponentType>(entity)) {
			registry.remove<ComponentType>(entity);
		}
	}

	template<typename ComponentType>
	Grindstone::Reflection::TypeDescriptor_Struct GetComponentReflectionData() {
		return ComponentType::reflectionInfo;
	}

	template<typename ComponentType>
	bool TryGetComponent(entt::registry& registry, entt::entity entity, void*& outComponent) {
		ComponentType* foundComp = registry.try_get<ComponentType>(entity);
		outComponent = foundComp;

		return foundComp != nullptr;
	}

	template<typename ComponentType>
	bool HasComponent(entt::registry& registry, entt::entity entity) {
		return registry.all_of<ComponentType>(entity);
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
