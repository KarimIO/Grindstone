#pragma once

#include <entt/entt.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

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

	template<typename T, typename = void>
	struct has_clone : std::false_type {};

	template<typename T>
	struct has_clone<T, std::void_t<decltype(std::declval<const T&>().Clone(
		std::declval<Grindstone::WorldContextSet&>(), std::declval<entt::entity>()))>> : std::true_type {};

	template<typename T>
	constexpr bool has_clone_v = has_clone<T>::value;

	template<typename ComponentType>
	void CopyRegistryComponents(WorldContextSet& dst, WorldContextSet& src) {
		entt::registry& srcRegistry = src.GetEntityRegistry();
		entt::registry& dstRegistry = dst.GetEntityRegistry();

		auto srcEntities = srcRegistry.view<ComponentType>();
		if constexpr (has_clone_v<ComponentType>) {
			for (entt::entity entityID : srcEntities) {
				ComponentType& srcComponent = srcRegistry.get<ComponentType>(entityID);
				dstRegistry.emplace_or_replace<ComponentType>(entityID, srcComponent.Clone(dst, entityID));
			}
		}
		else {
			for (entt::entity entityID : srcEntities) {
				dstRegistry.emplace_or_replace<ComponentType>(entityID, srcRegistry.get<ComponentType>(entityID));
			}
		}
	}
}
