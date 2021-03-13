#pragma once

#include <unordered_map>
#include <string>

#include "System.hpp"
#include "ComponentArray.hpp"
#include "Controller.hpp"

namespace Grindstone {
	namespace ECS {
		using SystemFactory = ISystem*(*)(Scene*);
		using ComponentFactory = IComponentArray*(*)();
		class Core {
		public:
			void registerSystem(const char* name, SystemFactory factory);
			void registerComponentType(const char *name, ComponentFactory factory);

			size_t getComponentTypeCount();
			size_t getSystemCount();

			void registerController(ECS::Controller &controller);

			IComponentArray *createComponentArray(const char*);
			ISystem *createSystem(const char*, Scene* scene);
			IComponentArray* createComponentArray(size_t i);
			ISystem* createSystem(size_t i, Scene* scene);
		private:
			std::unordered_map<std::string, SystemFactory> systemFactories;
			std::unordered_map<std::string, ComponentFactory> componentFactories;
		};
	}
}