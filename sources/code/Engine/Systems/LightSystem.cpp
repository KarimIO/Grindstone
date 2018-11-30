#include "LightSystem.hpp"

LightComponent::LightComponent(GameObjectHandle object_handle, ComponentHandle id) {
}

LightSystem::LightSystem() : System(COMPONENT_LIGHT) {
}

void LightSystem::update(double dt) {
	// Culling

	// CalculateView

	// Render
}

LightSubSystem::LightSubSystem() : SubSystem(COMPONENT_CAMERA) {
}

ComponentHandle LightSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	return ComponentHandle();
}

LightComponent & LightSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

void LightSubSystem::removeComponent(ComponentHandle handle) {
}

LightSubSystem::~LightSubSystem() {
}
