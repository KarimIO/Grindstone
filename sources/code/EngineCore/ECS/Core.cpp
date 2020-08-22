#include "Core.hpp"

namespace Grindstone {
	namespace ECS {
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