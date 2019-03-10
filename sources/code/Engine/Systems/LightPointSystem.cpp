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
	auto &component = components_.back();
		
	if (params.HasMember("color")) {
		auto color = params["color"].GetArray();
		component.properties_.color.x = color[0].GetFloat();
		component.properties_.color.y = color[1].GetFloat();
		component.properties_.color.z = color[2].GetFloat();
	}
	
	if (params.HasMember("brightness")) {
		float brightness = params["brightness"].GetFloat();
		component.properties_.power = brightness;
	}
	
	if (params.HasMember("radius")) {
		float radius = params["radius"].GetFloat();
		component.properties_.attenuationRadius = radius;
	}
	
	if (params.HasMember("castshadow")) {
		bool castshadow = params["castshadow"].GetBool();
		component.properties_.shadow = castshadow;
	}

	return component_handle;
}

LightPointComponent & LightPointSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

size_t LightPointSubSystem::getNumComponents() {
	return components_.size();
}

void LightPointSubSystem::removeComponent(ComponentHandle handle) {
}

LightPointSubSystem::~LightPointSubSystem() {
}
