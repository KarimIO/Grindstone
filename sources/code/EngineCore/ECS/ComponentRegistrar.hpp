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
			void registerComponent(const char* name) {
				registerComponent(name, {
					&ECS::createTagComponent<T>,
					&ECS::tryGetComponent<T>,
					&ECS::getComponentReflectionData<T>
				});
			}
			void registerComponent(const char *name, ComponentFunctions componentFunctions);
			void* createComponent(const char *name, entt::registry& registry, ECS::Entity entity);
			bool tryGetComponent(const char *name, entt::registry& registry, ECS::Entity entity, void*& outComponent);
			bool tryGetComponentReflectionData(const char *name, Grindstone::Reflection::TypeDescriptor_Struct& outReflectionData);

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
