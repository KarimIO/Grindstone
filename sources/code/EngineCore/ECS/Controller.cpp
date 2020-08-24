#include "Controller.hpp"
#include "Core.hpp"

namespace Grindstone {
	namespace ECS {
		Controller::Controller(Scene *scene) : scene_(scene), core_(nullptr) {
		}

		Entity Controller::createEntity() {
			static Entity next = 0;
			return ++next;
		}

		void Controller::removeEntity(Entity entity) {
			for (auto m : component_map_) {
				m.second->remove(entity);
			}
		}

		void Controller::registerSystem(const char* type, ISystem*sys) {
			system_map_[type] = sys;
		}

		void Controller::registerComponentType(const char* type, IComponentArray* carr) {
			component_map_[type] = carr;
		}

		void* Controller::createComponent(Entity entity, const char* component_name) {
			auto it = component_map_.find(component_name);
			if (it != component_map_.end()) {
				return it->second->createGeneric(entity);
			}
			else {
				std::string st = std::string("Controller::createComponent - Invalid component type: ") + component_name;
				throw std::runtime_error(st);
			}

			return nullptr;
		}

		Scene* Controller::getScene() {
			return scene_;
		}

		void Controller::update() {
			for (auto system : system_map_) {
				system.second->update();
			}
		}
	}
}
