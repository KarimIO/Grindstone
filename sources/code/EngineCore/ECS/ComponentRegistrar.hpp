#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <string>

#include "EngineCore/ECS/EntityHandle.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "ComponentFunctions.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar {
		public:
			template<typename T>
			void RegisterComponent(SetupComponentFn setupComponentFn = nullptr) {
				RegisterComponent(T::GetComponentName(), {
					setupComponentFn,
					&ECS::CreateComponent<T>,
					&ECS::RemoveComponent<T>,
					&ECS::HasComponent<T>,
					&ECS::TryGetComponent<T>,
					&ECS::GetComponentReflectionData<T>
				});
			}
			virtual void RegisterComponent(const char *name, ComponentFunctions componentFunctions);
			virtual void* CreateComponentWithSetup(const char* name, ECS::Entity& entity);
			virtual void* CreateComponent(const char *name, ECS::Entity& entity);
			virtual void RemoveComponent(const char *name, ECS::Entity& entity);
			virtual bool HasComponent(const char* name, ECS::Entity& entity);
			virtual bool TryGetComponent(const char *name, ECS::Entity&, void*& outComponent);
			virtual bool TryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData);
			virtual void SetupComponent(const char* componentType, ECS::Entity& entity, void* componentPtr);

			using ComponentMap = std::unordered_map<std::string, ComponentFunctions>;
			virtual ComponentMap::iterator begin();
			virtual ComponentMap::const_iterator begin() const;
			virtual ComponentMap::iterator end();
			virtual ComponentMap::const_iterator end() const;
		private:
			ComponentMap componentFunctionsList;
		};
	}
}
