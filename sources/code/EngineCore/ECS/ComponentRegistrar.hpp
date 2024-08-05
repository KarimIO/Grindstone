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
		template<typename ComponentType>
		void RegisterComponent(SetupComponentFn setupComponentFn = nullptr, DestroyComponentFn destroyComponentFn = nullptr) {
			RegisterComponent(ComponentType::GetComponentName(), {
				setupComponentFn,
				destroyComponentFn,
				&ECS::CreateComponent<ComponentType>,
				&ECS::RemoveComponent<ComponentType>,
				&ECS::HasComponent<ComponentType>,
				&ECS::TryGetComponent<ComponentType>,
				&ECS::GetComponentReflectionData<ComponentType>,
				&ECS::CopyRegistryComponents<ComponentType>
			});
		}

		template<typename ComponentType>
		void UnregisterComponent() {
			GetEntityRegistry().clear<ComponentType>();

			auto comp = componentFunctionsList.find(ComponentType::GetComponentName());
			if (comp != componentFunctionsList.end()) {
				componentFunctionsList.erase(comp);
			}
		}

		virtual entt::registry& GetEntityRegistry();
		virtual void CopyRegistry(entt::registry& to, entt::registry& from);
		virtual void CallCreateOnRegistry(entt::registry& registry);
		virtual void CallDestroyOnRegistry(entt::registry& registry);
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
