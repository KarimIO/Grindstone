#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <string>

#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "ComponentFunctions.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar {
		public:
			template<typename T>
			void RegisterComponent() {
				RegisterComponent(T::getComponentName(), {
					&ECS::CreateComponent<T>,
					&ECS::RemoveComponent<T>,
					&ECS::HasComponent<T>,
					&ECS::TryGetComponent<T>,
					&ECS::GetComponentReflectionData<T>
				});
			}
			void RegisterComponent(const char *name, ComponentFunctions componentFunctions);
			virtual void* CreateComponent(const char *name, entt::registry& registry, ECS::EntityHandle entity);
			virtual void RemoveComponent(const char *name, entt::registry& registry, ECS::EntityHandle entity);
			virtual bool HasComponent(const char* name, entt::registry& registry, ECS::EntityHandle entity);
			virtual bool TryGetComponent(const char *name, entt::registry& registry, ECS::EntityHandle entity, void*& outComponent);
			virtual bool TryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData);

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
