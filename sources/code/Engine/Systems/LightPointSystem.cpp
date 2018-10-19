#include "LightPointSystem.hpp"

LightPointComponent::LightPointComponent(ComponentHandle id) {
	component_type_ = COMPONENT_LIGHT_POINT;
	id_ = id;
}

Component *LightPointSystem::addComponent() {
	components_.emplace_back(components_.size());
	return &components_.back();
}

void LightPointSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

void LightPointSystem::update(double dt) {
	for (auto &component : components_) {
		for (int i = 0; i < 6; ++i) {
			// Culling
			
			// CalculateView

			// Render
		}
	}
}