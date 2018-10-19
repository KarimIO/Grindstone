#include "TransformSystem.hpp"
#include <glm/gtx/transform.hpp>

TransformComponent::TransformComponent(ComponentHandle id) :
	scale_(glm::vec3(1, 1, 1)) {
	component_type_ = COMPONENT_TRANSFORM;
	id_ = id;
}

Component *TransformSystem::addComponent() {
	components_.emplace_back(components_.size());
	return &components_.back();
}

void TransformSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

void TransformSystem::update(double dt) {
	for (auto &component : components_) {
		// For every component, generate the Model Matrix:

		// Start with an Identity Matrix
		component.model_ = glm::mat4(1);

		// Move in to the correct position
		component.model_ = glm::translate(component.model_, getPosition(component.id_));
		
		// Scale the Model
		component.model_ = glm::scale(component.model_, getScale(component.id_));

		// Rotate the Model along its axis
		glm::vec3 angles = getAngles(component.id_);
		float maxRot = (std::fmax(std::fmax(angles.x, angles.y), angles.z));
		if (maxRot > 0)
			component.model_ = glm::rotate(component.model_, maxRot, angles / maxRot);
	}
}

glm::vec3 TransformSystem::getForward(ComponentHandle handle) {
	glm::vec3 ang = getAngles(handle);
	return glm::vec3(
		glm::cos(ang.x) * glm::sin(ang.y),
		glm::sin(ang.x),
		glm::cos(ang.x) * glm::cos(ang.y)
	);
}

glm::vec3 TransformSystem::getRight(ComponentHandle handle) {
	glm::vec3 ang = getAngles(handle);
	return glm::vec3(
		glm::sin(ang.y - 3.14159f / 2.0f),
		0,
		glm::cos(ang.y - 3.14159f / 2.0f)
	);
}

glm::vec3 TransformSystem::getUp(ComponentHandle handle) {
	return glm::cross(getRight(handle), getForward(handle));
}

glm::vec3 TransformSystem::getAngles(ComponentHandle handle) {
	return getComponent(handle).angles_;
}

glm::vec3 TransformSystem::getPosition(ComponentHandle handle) {
	return getComponent(handle).position_;
}

glm::vec3 TransformSystem::getVelocity(ComponentHandle handle) {
	return getComponent(handle).velocity_;
}

glm::vec3 TransformSystem::getScale(ComponentHandle handle) {
	return getComponent(handle).scale_;
}

glm::mat4x4 & TransformSystem::getModelMatrix(ComponentHandle handle) {
	return getComponent(handle).model_;
}

inline TransformComponent & TransformSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}
