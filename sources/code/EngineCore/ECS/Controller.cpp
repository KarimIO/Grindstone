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

		void Controller::registerSystem(const char* type) {
			system_map_[type] = core_->createSystem(type, scene_);
		}

		void Controller::registerComponentType(const char* type) {
			component_map_[type] = core_->createComponentArray(type);
		}

		void Controller::update() {
			for (auto system : system_map_) {
				system.second->update();
			}
		}
	}
}
