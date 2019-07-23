#include "ControllerSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Core/Scene.hpp"
#include "../Core/Space.hpp"

#include "TransformSystem.hpp"

#include "Utilities/Logger.hpp"

ControllerSubSystem::ControllerSubSystem(Space *space) : SubSystem(COMPONENT_CONTROLLER, space) {
}

ComponentHandle ControllerSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	return component_handle;
}

ComponentHandle ControllerSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);

	setComponent(component_handle, params);

	return component_handle;
}

void ControllerSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];
}

ControllerComponent & ControllerSubSystem::getComponent(ComponentHandle handle) {
	return components_[handle];
}

void ControllerSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
}

void ControllerSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

ControllerSubSystem::~ControllerSubSystem() {
}

void ControllerSystem::update(double dt) {
	auto &scenes = engine.getScenes();
	for (auto scene : scenes) {
		for (auto space : scene->spaces_) {
			ControllerSubSystem *subsystem = (ControllerSubSystem *)space->getSubsystem(system_type_);
			for (auto &component : subsystem->components_) {
				component.update(dt);
			}
		}
	}
}

ControllerSystem::ControllerSystem() : System(COMPONENT_CONTROLLER) {}

ControllerComponent::ControllerComponent(GameObjectHandle object_handle, ComponentHandle handle) : Component(COMPONENT_CONTROLLER, object_handle, handle) {
	input.SetInputControlFile("cfgs/player.cfg");
	input.BindAxis("MoveForward", this, &ControllerComponent::MoveForwardBack);
	input.BindAxis("MoveSide", this, &ControllerComponent::MoveSide);
	input.BindAxis("MoveVertical", this, &ControllerComponent::MoveVertical);

	input.BindAxis("TurnPitch", this, &ControllerComponent::TurnPitch);
	input.BindAxis("TurnYaw", this, &ControllerComponent::TurnYaw);

	input.BindAction("ZoomIn", this, &ControllerComponent::ZoomIn);
	input.BindAction("ZoomOut", this, &ControllerComponent::ZoomOut);

	input.BindAction("Run", this, &ControllerComponent::RunStart, KEY_PRESSED);
	input.BindAction("Run", this, &ControllerComponent::RunStop, KEY_RELEASED);

	ghost_mode_ = true;
	no_collide_ = true;
	speed_modifier_ = 6.0f;
	sensitivity_ = engine.getSettings()->mouse_sensitivity_;
}

void ControllerComponent::update(double dt) {
	ComponentHandle transform_id = engine.getScene(0)->spaces_[0]->getObject(game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
	auto &trans = getTransform()->getComponent(transform_id);

	if (!ghost_mode_)
		velocity_.y -= 9.81f * (float)dt;

	velocity_.x *= 0.85f * (1.0f - dt);
	velocity_.y *= 0.85f * (1.0f - dt);
	velocity_.z *= 0.85f * (1.0f - dt);

	if (ghost_mode_)
		trans.position_ += velocity_ * (float)dt;

	if (!no_collide_) {
		if (trans.position_.y < 0.0) {
			trans.position_.y = 0.0;
		}
	}
}

TransformSubSystem *ControllerComponent::getTransform() {
	auto *space = engine.getScene(0)->spaces_[0];
	TransformSubSystem *transform = (TransformSubSystem *)(space->getSubsystem(COMPONENT_TRANSFORM));

	return transform;
}

size_t ControllerSubSystem::getNumComponents() {
	return components_.size();
}

void ControllerComponent::MoveForwardBack(double scale) {
	auto trans_system = getTransform();
	ComponentHandle transform_id = engine.getScene(0)->spaces_[0]->getObject(game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
	auto &trans = trans_system->getComponent(transform_id);

	glm::vec3 f = 20.0f * float(scale * speed_modifier_) * trans_system->getForward(transform_id);

	/*if (physID) {
		CPhysics *phys = &engine.physicsSystem.components[physID];
		phys->ApplyCentralForce(f);
	}
	else {*/
		velocity_ += f;
	//}
}

void ControllerComponent::MoveSide(double scale) {
	auto trans_system = getTransform();
	ComponentHandle transform_id = engine.getScene(0)->spaces_[0]->getObject(game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
	auto &trans = trans_system->getComponent(transform_id);

	glm::vec3 f = 5.0f * float(scale * speed_modifier_) * trans_system->getRight(transform_id);

	/*if (physID) {
		CPhysics *phys = &engine.physicsSystem.components[physID];
		phys->ApplyCentralForce(f);
	}
	else {*/
		velocity_ += f;
	//}
}

void ControllerComponent::MoveVertical(double scale) {
	auto trans_system = getTransform();
	ComponentHandle transform_id = engine.getScene(0)->spaces_[0]->getObject(game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
	auto &trans = trans_system->getComponent(transform_id);

	if (ghost_mode_) {
		glm::vec3 f = 5.0f * float(scale * speed_modifier_) * trans_system->getUp(transform_id);

		velocity_ += f;
	}
}

void ControllerComponent::TurnPitch(double scale) {
	if (!engine.edit_mode_ || engine.edit_is_simulating_) {
		auto trans_system = getTransform();
		ComponentHandle transform_id = engine.getScene(0)->spaces_[0]->getObject(game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
		auto &trans = trans_system->getComponent(transform_id);

		trans.angles_.x += float(sensitivity_ * scale);

		if (trans.angles_.x < -2.4f / 2)	trans.angles_.x = -2.4f / 2;
		if (trans.angles_.x > 3.14f / 2)	trans.angles_.x = 3.14f / 2;
	}
}

void ControllerComponent::TurnYaw(double scale) {
	if (!engine.edit_mode_ || engine.edit_is_simulating_) {
		auto trans_system = getTransform();
		ComponentHandle transform_id = engine.getScene(0)->spaces_[0]->getObject(game_object_handle_).getComponentHandle(COMPONENT_TRANSFORM);
		auto &trans = trans_system->getComponent(transform_id);

		trans.angles_.y += float(sensitivity_ * scale);
	}
}

void ControllerComponent::ZoomIn(double scale) {
	auto &fov = engine.getSettings()->fov_;
	/*fov -= 0.05f;
	if (engine.settings.fov < 0.4f)
		engine.settings.fov = 0.4f;*/
}

void ControllerComponent::ZoomOut(double scale) {
	auto &fov = engine.getSettings()->fov_;
	/*engine.settings.fov += 0.05f;
	if (engine.settings.fov > 1.57f)
		engine.settings.fov = 1.57f;*/
}

void ControllerComponent::RunStart(double scale) {
	speed_modifier_ = ghost_mode_ ? 10.0f : 6.0f;
}

void ControllerComponent::RunStop(double scale) {
	speed_modifier_ = ghost_mode_ ? 6.0f : 4.0f;
}