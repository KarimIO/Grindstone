#include "TransformSystem.hpp"
#include "Core/Engine.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include <glm/gtx/transform.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>

using namespace rapidjson;

TransformComponent::TransformComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_CAMERA, object_handle, handle),
	scale_(glm::vec3(1, 1, 1)) {
}

TransformSubSystem::TransformSubSystem(Space *space) : SubSystem(COMPONENT_TRANSFORM, space) {
}

ComponentHandle TransformSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

void TransformSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

TransformSystem::TransformSystem() : System(COMPONENT_TRANSFORM) {
}

void TransformSystem::update() {
	auto scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			TransformSubSystem *subsystem = (TransformSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				// For every component, generate the Model Matrix:
				// - Start with an Identity Matrix
				component.model_ = glm::mat4(1);

				// - Move in to the correct position
				component.model_ = glm::translate(component.model_, subsystem->getPosition(component.handle_));

				// - Scale the Model
				auto &s = component.scale_;
				component.model_ = glm::scale(component.model_, s);

				// - Rotate the Model along its axis
				glm::vec3 angles = subsystem->getAngles(component.handle_);
				float maxRot = (std::fmax(std::fmax(angles.x, angles.y), angles.z));
				if (maxRot > 0)
					component.model_ = glm::rotate(component.model_, maxRot, angles / maxRot);
			}
		}
	}
}

glm::vec3 TransformSubSystem::getForward(ComponentHandle handle) {
	glm::vec3 ang = getAngles(handle);
	return glm::vec3(
		glm::cos(ang.x) * glm::sin(ang.y),
		glm::sin(ang.x),
		glm::cos(ang.x) * glm::cos(ang.y)
	);
}

glm::vec3 TransformSubSystem::getRight(ComponentHandle handle) {
	glm::vec3 ang = getAngles(handle);
	return glm::vec3(
		glm::sin(ang.y - 3.14159f / 2.0f),
		0,
		glm::cos(ang.y - 3.14159f / 2.0f)
	);
}

glm::vec3 TransformSubSystem::getUp(ComponentHandle handle) {
	return glm::cross(getRight(handle), getForward(handle));
}

glm::vec3 TransformSubSystem::getAngles(ComponentHandle handle) {
	return components_[handle].angles_;
}

glm::vec3 TransformSubSystem::getPosition(ComponentHandle handle) {
	return components_[handle].position_;
}

glm::vec3 TransformSubSystem::getScale(ComponentHandle handle) {
	return components_[handle].scale_;
}

glm::mat4x4 & TransformSubSystem::getModelMatrix(ComponentHandle handle) {
	return components_[handle].model_;
}

TransformComponent &TransformSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

Component * TransformSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t TransformSubSystem::getNumComponents() {
	return components_.size();
}

TransformSubSystem::~TransformSubSystem() {

}

REFLECT_STRUCT_BEGIN(TransformComponent, TransformSystem, COMPONENT_TRANSFORM)
	REFLECT_STRUCT_MEMBER(position_)
	REFLECT_STRUCT_MEMBER(angles_)
	REFLECT_SUBCATS_START()
		REFLECT_SUBCAT_START("My Category")
			REFLECT_STRUCT_MEMBER(scale_)
		REFLECT_SUBCAT_END()
	REFLECT_SUBCATS_END()
REFLECT_STRUCT_END()
