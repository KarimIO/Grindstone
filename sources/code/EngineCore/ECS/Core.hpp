#pragma once

#include <unordered_map>
#include <string>

#include "System.hpp"
#include "ComponentArray.hpp"

namespace Grindstone {
	namespace ECS {
		using SystemFactory = ISystem*(*)(Scene*s);
		using ComponentFactory = IComponentArray*(*)();
		class Core {
		public:
			void registerSystem(const char* name, SystemFactory factory) {
				// T::static_system_type_ = 0;
				system_factories_[name] = factory;
			}
			
			void registerComponentType(const char *name, ComponentFactory factory) {
				// T::component_type_ = 0;
				// ComponentArray<T>::static_component_type_ = 0;
				component_array_factories_[name] = factory;
			}

			size_t getComponentTypeCount();
			size_t getSystemCount();

			IComponentArray *createComponentArray(const char*);
			ISystem *createSystem(const char*, Scene* scene);
			IComponentArray* createComponentArray(size_t i);
			ISystem* createSystem(size_t i, Scene* scene);
		private:
			std::unordered_map<std::string, SystemFactory> system_factories_;
			std::unordered_map<std::string, ComponentFactory> component_array_factories_;
		};
	}
}