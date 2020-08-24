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
			virtual Entity createEntity();
			virtual void removeEntity(Entity entity);

			virtual void registerSystem(const char* type, ISystem*sys);
			virtual void registerComponentType(const char *type, IComponentArray* carr);

			virtual void* createComponent(Entity entity, const char* component_name);

			Scene* getScene();

			void update();
		private:
			std::unordered_map<std::string , IComponentArray* > component_map_;
			std::unordered_map<std::string, ISystem*> system_map_;
			Scene* scene_;
			ECS::Core* core_;
		};
	}
}
