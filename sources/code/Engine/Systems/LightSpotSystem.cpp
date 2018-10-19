#include "LightSpotSystem.hpp"

LightSpotComponent::LightSpotComponent(ComponentHandle id) {
	component_type_ = COMPONENT_LIGHT_SPOT;
	id_ = id;
}

Component *LightSpotSystem::addComponent() {
	components_.emplace_back(components_.size());
	return &components_.back();
}

void LightSpotSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

void LightSpotSystem::update(double dt) {
	for (auto &component : components_) {
		// Culling

		// CalculateView

		// Render
	}
}