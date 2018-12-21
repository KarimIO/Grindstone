#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"
#include "LightSpotSystem.hpp"

LightSpotComponent::LightSpotComponent(GameObjectHandle object_handle, ComponentHandle id) : Component(COMPONENT_LIGHT_SPOT, object_handle, id) {}

LightSpotSubSystem::LightSpotSubSystem(Space *space) : SubSystem(COMPONENT_LIGHT_SPOT, space) {}

LightSpotSystem::LightSpotSystem() : System(COMPONENT_LIGHT_SPOT) {}

void LightSpotSystem::update(double dt) {
	auto &scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			LightSpotSubSystem *subsystem = (LightSpotSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				// Culling

				// CalculateView

				// Render
			}
		}
	}
}

ComponentHandle LightSpotSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value & params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	//LightComponent &component = components_.back();

	return component_handle;
}

LightSpotComponent & LightSpotSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

void LightSpotSubSystem::removeComponent(ComponentHandle handle) {
}

LightSpotSubSystem::~LightSpotSubSystem() {
}
