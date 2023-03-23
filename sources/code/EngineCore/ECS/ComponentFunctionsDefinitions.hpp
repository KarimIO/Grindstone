#pragma once

#include "ComponentFunctions.hpp"

namespace Grindstone {
	namespace ECS {
		template<typename T>
		void* CreateComponent(ECS::Entity& entity) {
			return &entity.GetSceneEntityRegistry().emplace<T>(entity.GetHandle());
		}

		template<typename T>
		void RemoveComponent(ECS::Entity& entity) {
			auto& registry = entity.GetSceneEntityRegistry();
			auto entityHandle = entity.GetHandle();
			if (registry.any_of<T>(entityHandle)) {
				registry.remove<T>(entityHandle);
			}
		}

		template<typename T>
		Grindstone::Reflection::TypeDescriptor_Struct GetComponentReflectionData() {
			return T::reflectionInfo;
		}

		template<typename T>
		bool TryGetComponent(ECS::Entity& entity, void*& outComponent) {
			auto& registry = entity.GetSceneEntityRegistry();
			auto entityHandle = entity.GetHandle();
			if (registry.any_of<T>(entityHandle)) {
				outComponent = &registry.get<T>(entityHandle);
				return true;
			}

			outComponent = nullptr;
			return false;
		}

		template<typename T>
		bool HasComponent(ECS::Entity& entity) {
			auto& registry = entity.GetSceneEntityRegistry();
			auto entityHandle = entity.GetHandle();
			return registry.all_of<T>(entityHandle);
		}
	}
}
