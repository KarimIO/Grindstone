#pragma once

#include <unordered_map>
#include <string>
#include <entt/entt.hpp>

#include <EngineCore/ECS/EntityHandle.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>

#include "ComponentFunctions.hpp"
#include "ComponentFunctionsDefinitions.hpp"

using namespace Grindstone;

namespace Grindstone::ECS {
	class ComponentRegistrar {
	public:
		template<typename T>
		void RegisterComponent(SetupComponentFn setupComponentFn = nullptr, DestroyComponentFn destroyComponentFn = nullptr) {
			RegisterComponent(T::GetComponentName(), {
				setupComponentFn,
				destroyComponentFn,
				&ECS::CreateComponent<T>,
				&ECS::RemoveComponent<T>,
				&ECS::HasComponent<T>,
				&ECS::TryGetComponent<T>,
				&ECS::GetComponentReflectionData<T>
			});
		}

		template<typename ClassType>
		void UnregisterComponent() {
			GetEntityRegistry().clear<ClassType>();

			auto comp = componentFunctionsList.find(ClassType::GetComponentName());
			if (comp != componentFunctionsList.end()) {
				componentFunctionsList.erase(comp);
			}
		}

		virtual entt::registry& GetEntityRegistry();
		virtual void DestroyEntity(ECS::Entity entity);
		virtual void RegisterComponent(const char* name, ComponentFunctions componentFunctions);
		virtual void UnregisterComponent(const char* name);
		virtual void* CreateComponentWithSetup(const char* name, ECS::Entity entity);
		virtual void* CreateComponent(const char *name, ECS::Entity entity);
		virtual void RemoveComponent(const char *name, ECS::Entity entity);
		virtual bool HasComponent(const char* name, ECS::Entity entity);
		virtual bool TryGetComponent(const char *name, ECS::Entity entity, void*& outComponent);
		virtual bool TryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData);
		virtual void SetupComponent(const char* componentType, ECS::Entity entity, void* componentPtr);

		using ComponentMap = std::unordered_map<std::string, ComponentFunctions>;
		virtual ComponentMap::iterator begin();
		virtual ComponentMap::const_iterator begin() const;
		virtual ComponentMap::iterator end();
		virtual ComponentMap::const_iterator end() const;
	private:
		ComponentMap componentFunctionsList;
	};
}
