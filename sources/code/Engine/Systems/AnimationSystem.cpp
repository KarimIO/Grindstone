#include "Core/Engine.hpp"
#include "AnimationSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/ModelManager.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

AnimationComponent::AnimationComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_ANIMATION, object_handle, handle) {}

void AnimationComponent::play() {
	playing_ = true;
}

void AnimationComponent::pause() {
	playing_ = false;
}

void AnimationComponent::readNodeHeirarchy(double time, Animation::Node *node, glm::mat4x4 parent_transform) {
	// Find Keyframes
	glm::mat4 global_transform;
	glm::mat4 scaling_mat, rotating_mat, position_mat;
	glm::vec3 scaling;
	glm::vec3 position;
	glm::quat rotation;

	if (node->keyframe_.size() == 1) {
		auto key = node->keyframe_[0];
		scaling = key.scale_;
		position = key.position_;
		rotation = key.rotation_;
	}
	else {
		for (size_t i = 0; i < node->keyframe_.size() - 1; ++i) {
			auto key = node->keyframe_[i];
			auto next_key = node->keyframe_[i + 1];
			if (key.time_ > time) {
				float delta_time = next_key.time_ - key.time_;
				float factor = (time - key.time_) / delta_time;

				scaling = next_key.scale_ + (next_key.scale_ - key.scale_) * factor;
				position = next_key.position_ + (next_key.position_ - key.position_) * factor;
				rotation = glm::slerp(key.rotation_, next_key.rotation_, factor);
			}
		}
	}

	scaling_mat = glm::scale(scaling_mat, scaling);
	rotating_mat = glm::toMat4(rotation);
	position_mat = glm::translate(position_mat, position);

	// Combine the above transformations
	global_transform = parent_transform * position_mat * rotating_mat * scaling_mat;
	bones_animated_[node->bone_id_] = skeleton_->global_inverse_ * global_transform * skeleton_->skeleton_[node->bone_id_];
	
	for (auto child : node->children_) {
		readNodeHeirarchy(time, child, global_transform);
	}
}

void AnimationComponent::update(double dt) {
	// For every anim this component handles...
	for (auto &anim : animations_) {

		double animlen = anim.animation_->length_;
		// Iterate on time, and either loop or end.
		if (anim.loop_) {
			anim.current_time_ += std::fmod(animlen, anim.current_time_ + dt);
		}
		else {
			anim.current_time_ += dt;

			// Should we just pause at the end?
			if (anim.current_time_ >= animlen) {
				pause();
			}
		}

		readNodeHeirarchy(anim.current_time_, anim.animation_->root_, glm::mat4());
	}
}

// SYSTEMS

AnimationSubSystem::AnimationSubSystem(Space *space) : SubSystem(COMPONENT_ANIMATION, space) {
}

AnimationSubSystem::~AnimationSubSystem() {
}

ComponentHandle AnimationSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto component = components_.back();

	if (params.HasMember("path")) {
		auto path = params["path"].GetString();
		//component.path_ = path;
	}

	return component_handle;
}

void AnimationSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

AnimationComponent &AnimationSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

size_t AnimationSubSystem::getNumComponents() {
	return components_.size();
}

void AnimationSystem::update(double dt) {
}

AnimationSystem::AnimationSystem() : System(COMPONENT_CAMERA) {}
