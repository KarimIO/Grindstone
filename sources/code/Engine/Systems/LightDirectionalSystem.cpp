#include "LightDirectionalSystem.hpp"

LightDirectionalComponent::LightDirectionalComponent(ComponentHandle id) {
	component_type_ = COMPONENT_LIGHT_DIRECTIONAL;
	id_ = id;
}

Component *LightDirectionalSystem::addComponent() {
	components_.emplace_back(ComponentHandle(components_.size()));
	return &components_.back();
}

void LightDirectionalSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

void LightDirectionalSystem::update(double dt) {
	for (auto &component : components_) {
		// Culling

		// CalculateView

		// Render
	}
}