#include "Core.hpp"
#include "Controller.hpp"

namespace Grindstone {
	namespace ECS {

		void Core::registerController(ECS::Controller &controller) {
			Scene *scene = controller.getScene();

			for (auto &sys_factory : system_factories_) {
				auto sys = sys_factory.second(scene);
				controller.registerSystem(sys_factory.first.c_str(), sys);
			}

			for (auto &comp_factory : component_array_factories_) {
				auto comp = comp_factory.second();
				controller.registerComponentType(comp_factory.first.c_str(), comp);
			}
		}

		void Core::registerSystem(const char* name, SystemFactory factory) {
			system_factories_[name] = factory;
		}
		
		void Core::registerComponentType(const char *name, ComponentFactory factory) {
			component_array_factories_[name] = factory;
		}

		size_t Core::getComponentTypeCount() {
			return component_array_factories_.size();
		}

		size_t Core::getSystemCount() {
			return system_factories_.size();
		}

		IComponentArray* Core::createComponentArray(const char* str) {
			auto it = component_array_factories_.find(str);
			if (it != component_array_factories_.end()) {
				return it->second();
			}
			else {
				return nullptr;
			}
		}

		ISystem* Core::createSystem(const char*str, Scene *scene) {
			auto it = system_factories_.find(str);
			if (it != system_factories_.end()) {
				return it->second(scene);
			}
			else {
				return nullptr;
			}
		}

		IComponentArray* Core::createComponentArray(size_t i) {
			auto it = std::next(component_array_factories_.begin(), i);
			return it->second();
		}

		ISystem* Core::createSystem(size_t i, Scene* scene) {
			auto it = std::next(system_factories_.begin(), i);
			return it->second(scene);
		}
	}
}