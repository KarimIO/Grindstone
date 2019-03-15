#include "BaseSystem.hpp"

ComponentType getComponentType(std::string str) {
	for (int i = 0; i < NUM_COMPONENTS - 1; ++i) {
		if (std::string(component_names[i]) == str) {
			return ComponentType(i);
		}
	}

	return COMPONENT_BASE;
}

SubSystem::SubSystem(ComponentType type = COMPONENT_BASE, Space *space = nullptr) : system_type_(type), space_(space) {
}

ComponentType SubSystem::getSystemType() {
	return system_type_;
}

Component::Component() {}

Component::Component(ComponentType type, GameObjectHandle game_object_handle, ComponentHandle component_handle) :
	component_type_(type),
	game_object_handle_(game_object_handle),
	handle_(component_handle) {}

SubSystem::~SubSystem() {}

System::System(ComponentType type) : system_type_(type) {}

System::~System() {}