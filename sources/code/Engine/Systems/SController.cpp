#include "SController.hpp"
#include "../Core/Engine.hpp"

void SController::AddComponent(unsigned int entityID, unsigned int &target) {
	components.push_back(CController());
	components.back().Initialize(entityID);
	target = (unsigned int)(components.size() - 1);
}

void SController::update(double dt) {
	for (auto &c : components) {
		c.update(dt);
	}
}

void CController::update(double dt) {
	auto &entity = engine.entities[entityID];
	unsigned int transfID = entity.components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	if (!ghost_mode_)
		trans->velocity.y -= 9.81f * (float)dt;

	trans->velocity.x *= 0.85f * (1.0f - dt);
	trans->velocity.y *= 0.85f * (1.0f - dt);
	trans->velocity.z *= 0.85f * (1.0f - dt);

	if (ghost_mode_)
		trans->position += trans->velocity * (float)dt;

	if (!no_collide_) {
		if (trans->position.y < 0.0) {
			trans->position.y = 0.0;
		}
	}
}

void CController::Initialize(unsigned int _entityID) {
	this->entityID = _entityID;
	input.SetInputControlFile("cfgs/player.cfg");
	input.BindAxis("MoveForward", this, &CController::MoveForwardBack);
	input.BindAxis("MoveSide", this, &CController::MoveSide);
	input.BindAxis("MoveVertical", this, &CController::MoveVertical);
	
	input.BindAxis("TurnPitch", this, &CController::TurnPitch);
	input.BindAxis("TurnYaw", this, &CController::TurnYaw);
	
	input.BindAction("ZoomIn", this, &CController::ZoomIn);
	input.BindAction("ZoomOut", this, &CController::ZoomOut);
	
	input.BindAction("Run", this, &CController::RunStart, KEY_PRESSED);
	input.BindAction("Run", this, &CController::RunStop, KEY_RELEASED);
	
	ghost_mode_ = true;
	no_collide_ = true;
	speed_modifier_ = 8.0;
	sensitivity_ = engine.settings.mouse_sensitivity;
}

void CController::MoveForwardBack(double scale) {
	auto &entity = engine.entities[entityID];
	unsigned int transfID = entity.components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];
	unsigned int physID = entity.components_[COMPONENT_PHYSICS];

	glm::vec3 f = 20.0f * float(scale * speed_modifier_) * trans->GetForward();

	/*if (physID) {
		CPhysics *phys = &engine.physicsSystem.components[physID];
		phys->ApplyCentralForce(f);
	}
	else {*/
		trans->velocity += f;
	//}
}

void CController::MoveSide(double scale) {
	auto &entity = engine.entities[entityID];
	unsigned int transfID = entity.components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];
	unsigned int physID = entity.components_[COMPONENT_PHYSICS];

	glm::vec3 f = 5.0f * float(scale * speed_modifier_) * trans->GetRight();

	/*if (physID) {
		CPhysics *phys = &engine.physicsSystem.components[physID];
		phys->ApplyCentralForce(f);
	}
	else {*/
	trans->velocity += f;
	//}
}

void CController::MoveVertical(double scale) {
	unsigned int transfID = engine.entities[entityID].components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	if (ghost_mode_) {
		glm::vec3 f = 5.0f * float(scale * speed_modifier_) * trans->GetUp();

		trans->velocity += f;
	}
}

void CController::TurnPitch(double scale) {
	unsigned int transfID = engine.entities[entityID].components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	trans->angles.x += float(sensitivity_ * scale);

	if (trans->angles.x < -2.4f / 2)	trans->angles.x = -2.4f / 2;
	if (trans->angles.x > 3.14f / 2)	trans->angles.x = 3.14f / 2;
}

void CController::TurnYaw(double scale) {
	unsigned int transfID = engine.entities[entityID].components_[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];
	trans->angles.y += float(sensitivity_ * scale);
}

void CController::ZoomIn(double scale) {
	engine.settings.fov -= 0.05f;
	if (engine.settings.fov < 0.4f)
		engine.settings.fov = 0.4f;
}

void CController::ZoomOut(double scale) {
	engine.settings.fov += 0.05f;
	if (engine.settings.fov > 1.57f)
		engine.settings.fov = 1.57f;
}

void CController::RunStart(double scale) {
	speed_modifier_ = ghost_mode_ ? 10.0f : 9.0f;
}

void CController::RunStop(double scale) {
	speed_modifier_ = ghost_mode_ ? 6.0f : 5.0;
}
