#pragma once

#include <entt/entt.hpp>
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		using SetupComponentFn = void(*)(ECS::Entity& entity, void* componentPtr);
		using GetComponentReflectionDataFn = Grindstone::Reflection::TypeDescriptor_Struct(*)();
		using TryGetComponentFn = bool(*)(ECS::Entity& entity, void*& outEntity);
		using HasComponentFn = bool(*)(ECS::Entity& entity);
		using CreateComponentFn = void*(*)(ECS::Entity& entity);
		using RemoveComponentFn = void(*)(ECS::Entity& entity);

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
		
		class ComponentFunctions {
		public:
			SetupComponentFn SetupComponentFn = nullptr;
			CreateComponentFn CreateComponentFn = nullptr;
			RemoveComponentFn RemoveComponentFn = nullptr;
			HasComponentFn HasComponentFn = nullptr;
			TryGetComponentFn TryGetComponentFn = nullptr;
			GetComponentReflectionDataFn GetComponentReflectionDataFn = nullptr;
		};
	}
}
