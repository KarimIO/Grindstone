#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightDirectionalSystem.hpp"

LightDirectionalComponent::LightDirectionalComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_DIRECTIONAL, object_handle, id) {}

LightDirectionalSubSystem::LightDirectionalSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_DIRECTIONAL, space) {}

LightDirectionalSystem::LightDirectionalSystem() : System(COMPONENT_LIGHT_DIRECTIONAL) {}

void LightDirectionalSystem::update(double dt) {
	auto &scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			LightDirectionalSubSystem *subsystem = (LightDirectionalSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				// Culling

				// CalculateView

				// Render
			}
		}
	}
}

ComponentHandle LightDirectionalSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	//LightComponent &component = components_.back();

	return component_handle;
}

LightDirectionalComponent & LightDirectionalSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t LightDirectionalComponent::getNumComponents() {
	return 0; // components_.size();
}

void LightDirectionalSubSystem::removeComponent(ComponentHandle handle) {
}

LightDirectionalSubSystem::~LightDirectionalSubSystem() {
}
