#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightPointSystem.hpp"

LightPointComponent::LightPointComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_POINT, object_handle, id) {}

LightPointSubSystem::LightPointSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_POINT, space) {}

LightPointSystem::LightPointSystem() : System(COMPONENT_LIGHT_POINT) {}

void LightPointSystem::update(double dt) {
	auto &scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			LightPointSubSystem *subsystem = (LightPointSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				// Culling

				// CalculateView

				// Render
			}
		}
	}
}

ComponentHandle LightPointSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	//LightComponent &component = components_.back();

	return component_handle;
}

LightPointComponent & LightPointSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

void LightPointSubSystem::removeComponent(ComponentHandle handle) {
}

LightPointSubSystem::~LightPointSubSystem() {
}
