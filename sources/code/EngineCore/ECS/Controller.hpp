#pragma once

#include "Entity.hpp"
#include "Component.hpp"
#include "ComponentArray.hpp"
#include "System.hpp"

#include <unordered_map>
#include <iostream>

namespace Grindstone {
	namespace ECS {
		class Core;
		class Controller {
		public:
			Controller(Scene*);
			Entity createEntity();
			void removeEntity(Entity entity);

			void registerSystem(const char* type);
			void registerComponentType(const char *type);

			void* createComponent(Entity entity, const char* component_name) {
				auto it = component_map_.find(component_name);
				if (it != component_map_.end()) {
					return it->second->createGeneric(entity);
				}

				return nullptr;
			}

			void update();
		private:
			std::unordered_map<std::string , IComponentArray* > component_map_;
			std::unordered_map<std::string, ISystem*> system_map_;
			Scene* scene_;
			ECS::Core* core_;
		};
	}
}
