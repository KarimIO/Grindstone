#include "Core/Engine.hpp"
#include "AnimationSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include "../AssetManagers/ModelManager.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <fstream>
#include "Utilities/Logger.hpp"

void loadAnimation(std::string path, Skeleton *skeleton, Animation *animation) {
	std::ifstream input("../assets/animations/" + path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		throw std::runtime_error("Failed to open file: " + path + "!");
	}

	GRIND_LOG("Model reading from: {0}!", path);

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	auto bufferpos = buffer.data();

	if (buffer[0] != 'G' && buffer[1] != 'A' || buffer[2] != 'F') {
		throw std::runtime_error("Invalid File: Animation doesn't start with GAF");
	}

	animation->length_ = (float)bufferpos[3];

	bufferpos += 3 * sizeof(char) + sizeof(float);

	size_t numbones;

	// Copy global inverse
	memcpy((void *)&numbones, bufferpos, sizeof(size_t));

	// Go to bones
	bufferpos += sizeof(size_t);

	for (size_t i = 0; i < numbones; ++i) {
		animation->root_;
	}

	input.close();
}

AnimationComponent::AnimationComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_ANIMATION, object_handle, handle), animation_path_(""), skeleton_path_(""), playing_(false), skeleton_(nullptr) {
}

AnimationComponent::AnimationComponent(GameObjectHandle object_handle, ComponentHandle handle, std::string animation_path, std::string skeleton_path) :
	Component(COMPONENT_ANIMATION, object_handle, handle), animation_path_(animation_path), skeleton_path_(skeleton_path), playing_(false) {
	skeleton_ = new Skeleton();
	// Load Skeleton File
	GrindstoneAssetCommon::loadSkeleton(skeleton_path, skeleton_->global_inverse_, skeleton_->bone_info_);

	// Load Animation
	Animation *animation = nullptr;
	loadAnimation(animation_path, skeleton_, animation);
}

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

	// If there's only one keyframe, use the data for that keyframe
	if (node->keyframe_.size() == 1) {
		auto key = node->keyframe_[0];
		scaling = key.scale_;
		position = key.position_;
		rotation = key.rotation_;
	}
	else {
		// Otherwise, loop over all keyframes...
		for (size_t i = 0; i < node->keyframe_.size() - 1; ++i) {
			auto key = node->keyframe_[i];
			// ...and test if this is the appropriate keyframe.
			if (key.time_ > time) {
				// If so, use the transformation information of that keyframe
				auto next_key = node->keyframe_[i + 1];
				float delta_time = (float)(next_key.time_ - key.time_);
				float factor = (float)(time - key.time_) / delta_time;

				scaling = next_key.scale_ + (next_key.scale_ - key.scale_) * factor;
				position = next_key.position_ + (next_key.position_ - key.position_) * factor;
				rotation = glm::slerp(key.rotation_, next_key.rotation_, factor);
			}
		}
	}

	// Get the matrices
	scaling_mat		= glm::scale(scaling_mat, scaling);
	rotating_mat	= glm::toMat4(rotation);
	position_mat	= glm::translate(position_mat, position);

	// Combine the above transformations
	global_transform = parent_transform * position_mat * rotating_mat * scaling_mat;
	// bones_animated_[node->bone_id_] = skeleton_->global_inverse_ * global_transform * skeleton_->skeleton_[node->bone_id_];
	
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

ComponentHandle AnimationSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

AnimationSubSystem::~AnimationSubSystem() {
}

ComponentHandle AnimationSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();

	std::string animation_path, skeleton_path;

	if (params.HasMember("animation")) {
		animation_path = params["animation"].GetString();
	}

	if (params.HasMember("skeleton")) {
		skeleton_path = params["skeleton"].GetString();
	}

	components_.emplace_back(object_handle, component_handle, animation_path, skeleton_path);
	auto component = components_.back();

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

void AnimationSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
}

void AnimationSystem::update(double dt) {
}

AnimationSystem::AnimationSystem() : System(COMPONENT_CAMERA) {}
