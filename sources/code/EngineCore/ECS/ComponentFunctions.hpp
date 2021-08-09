#pragma once

#include <entt/entt.hpp>
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		using GetComponentReflectionDataFn = Grindstone::Reflection::TypeDescriptor_Struct(*)();
		using TryGetComponentFn = bool(*)(entt::registry& registry, entt::entity entity, void*& outEntity);
		using HasComponentFn = bool(*)(entt::registry& registry, entt::entity entity);
		using CreateComponentFn = void*(*)(entt::registry& registry, entt::entity entity);
		using RemoveComponentFn = void(*)(entt::registry& registry, entt::entity entity);

		template<typename T>
		void* CreateComponent(entt::registry& registry, entt::entity entity) {
			return &registry.emplace<T>(entity);
		}

		template<typename T>
		void RemoveComponent(entt::registry& registry, entt::entity entity) {
			if (registry.has<T>(entity)) {
				registry.remove<T>(entity);
			}
		}

		template<typename T>
		Grindstone::Reflection::TypeDescriptor_Struct GetComponentReflectionData() {
			return T::reflectionInfo;
		}

		template<typename T>
		bool TryGetComponent(entt::registry& registry, entt::entity entity, void*& outEntity) {
			if (registry.has<T>(entity)) {
				outEntity = &registry.get<T>(entity);
				return true;
			}

			outEntity = nullptr;
			return false;
		}

		template<typename T>
		bool HasComponent(entt::registry& registry, entt::entity entity) {
			return registry.has<T>(entity);
		}
		
		class ComponentFunctions {
		public:
			CreateComponentFn CreateComponentFn;
			RemoveComponentFn RemoveComponentFn;
			HasComponentFn HasComponentFn;
			TryGetComponentFn TryGetComponentFn;
			GetComponentReflectionDataFn GetComponentReflectionDataFn;
		};
	}
}
