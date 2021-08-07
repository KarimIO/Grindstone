#pragma once

#include <entt/entt.hpp>
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		using GetComponentReflectionDataFn = Grindstone::Reflection::TypeDescriptor_Struct(*)();
		using TryGetComponentFn = bool(*)(entt::registry& registry, entt::entity entity, void*& outEntity);
		using CreateComponentFn = void*(*)(entt::registry& registry, entt::entity entity);
		using DeleteComponentFn = void(*)(entt::registry& registry, entt::entity entity);

		template<typename T>
		void* createComponent(entt::registry& registry, entt::entity entity) {
			return &registry.emplace<T>(entity);
		}

		template<typename T>
		void deleteComponent(entt::registry& registry, entt::entity entity) {
			if (registry.has<T>(entity)) {
				registry.remove<T>(entity);
			}
		}

		template<typename T>
		Grindstone::Reflection::TypeDescriptor_Struct getComponentReflectionData() {
			return T::reflectionInfo;
		}

		template<typename T>
		bool tryGetComponent(entt::registry& registry, entt::entity entity, void*& outEntity) {
			if (registry.has<T>(entity)) {
				outEntity = &registry.get<T>(entity);
				return true;
			}

			outEntity = nullptr;
			return false;
		}
		
		class ComponentFunctions {
		public:
			CreateComponentFn createComponentFn;
			DeleteComponentFn deleteComponentFn;
			TryGetComponentFn tryGetComponentFn;
			GetComponentReflectionDataFn getComponentReflectionDataFn;
		};
	}
}
