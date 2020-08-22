#pragma once

#include <unordered_map>
#include <string>

#include "System.hpp"
#include "ComponentArray.hpp"

namespace Grindstone {
	namespace ECS {
		class Core {
		public:
			template<typename T>
			void registerSystem(const char* name) {
				// T::static_system_type_ = 0;
				system_factories_[name] = T::createSystem;
			}
			template<typename T>
			void registerComponentType(const char *name) {
				// T::component_type_ = 0;
				// ComponentArray<T>::static_component_type_ = 0;
				auto f = ComponentArray<T>::createComponentArray;
				component_array_factories_[name] = f;
			}

			size_t getComponentTypeCount();
			size_t getSystemCount();

			IComponentArray *createComponentArray(const char*);
			ISystem *createSystem(const char*, Scene* scene);
			IComponentArray* createComponentArray(size_t i);
			ISystem* createSystem(size_t i, Scene* scene);
		private:
			std::unordered_map<std::string, ISystem*(*)(Scene*s)> system_factories_;
			std::unordered_map<std::string, IComponentArray*(*)()> component_array_factories_;
		};
	}
}