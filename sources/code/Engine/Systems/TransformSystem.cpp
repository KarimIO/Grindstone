#include "TransformSystem.hpp"
#include "Core/Engine.hpp"
#include "Core/Scene.hpp"
#include "Core/Space.hpp"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>

using namespace rapidjson;

TransformComponent::TransformComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_TRANSFORM, object_handle, handle),
	local_scale_(glm::vec3(1, 1, 1)), parent_(nullptr) {
}

void TransformComponent::setParent(TransformComponent* tc) {
	parent_ = tc;
}

void TransformComponent::addChild(TransformComponent* tc) {
	children_.push_back(tc);
}

void TransformComponent::removeChild(uint32_t i) {
	children_.erase(children_.begin() + i);
}

TransformComponent *TransformComponent::getChild(uint32_t i) {
	return children_[i];
}

void TransformComponent::setLocalPosition(glm::vec3 pos) {
	local_position_ = pos;
}

void TransformComponent::setPosition(glm::vec3 pos) {
	position_ = pos;
}

void TransformComponent::setLocalRotation(glm::quat rot) {
	local_rotation_ = rot;
}

void TransformComponent::setRotation(glm::quat rot) {
	rotation_ = rot;
}

void TransformComponent::setLocalScale(glm::vec3 scale) {
	local_scale_ = scale;
}

glm::vec3 TransformComponent::getLocalPosition() {
	return local_position_;
}

glm::vec3 TransformComponent::getPosition() {
	return position_;
}

glm::quat TransformComponent::getLocalRotation() {
	return local_rotation_;
}

glm::quat TransformComponent::getRotation() {
	return rotation_;
}

glm::vec3 TransformComponent::getLocalScale() {
	return local_scale_;
}

glm::vec3 TransformComponent::getForward() {
	return forward_;
}

glm::vec3 TransformComponent::getRight() {
	return right_;
}

glm::vec3 TransformComponent::getUp() {
	return up_;
}

glm::mat4x4& TransformComponent::getModelMatrix() {
	return model_;
}

TransformSubSystem::TransformSubSystem(Space *space) : SubSystem(COMPONENT_TRANSFORM, space) {
}

ComponentHandle TransformSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	//TransformComponent* tc = &components_.back();

	//root_components_.push_back(tc);

	return component_handle;
}

void TransformSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

TransformSystem::TransformSystem() : System(COMPONENT_TRANSFORM) {
}

void TransformSystem::update() {
	GRIND_PROFILE_FUNC();
	for (auto space : engine.getSpaces()) {
		TransformSubSystem *subsystem = (TransformSubSystem *)space->getSubsystem(system_type_);
		glm::vec3 position;
		glm::quat rotation;
		glm::mat4 model;
		
		for (auto&component : subsystem->components_) {
			// For every component, generate the Model Matrix:
			// - Move in to the correct position
			component.local_model_ = glm::translate(glm::mat4(1), component.local_position_);

			// - Scale the Model
			auto &s = component.local_scale_;
			component.local_model_ = glm::scale(component.local_model_, s);

			// - Rotate the Model along its axis
			component.local_model_ = glm::toMat4(component.local_rotation_) * component.local_model_;

			component.position_ = component.local_position_; // model* glm::vec4(component.local_position_, 1.0f);
			component.rotation_ = component.local_rotation_;
			// component.rotation_	= model * component.local_rotation_;
			// model = model * component.local_model_;
			component.model_ = component.local_model_;

			// Set Directions
			component.forward_	= component.rotation_ * glm::vec3(0.0f, 0.0f, 1.0f);
			component.up_		= component.rotation_ * glm::vec3(0.0f, 1.0f, 0.0f);
			component.right_	= component.rotation_ * glm::vec3(1.0f, 0.0f, 0.0f);
		}
	}
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
	REFLECT_STRUCT_MEMBER_D(local_position_, "Position", "position", reflect::Metadata::SaveSetAndView, nullptr)
	REFLECT_STRUCT_MEMBER_D(local_rotation_, "Rotation", "rotation", reflect::Metadata::SaveSetAndView, nullptr)
	REFLECT_STRUCT_MEMBER_D(local_scale_, "Scale", "scale", reflect::Metadata::SaveSetAndView, nullptr)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
