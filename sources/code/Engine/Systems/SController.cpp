#include "CController.h"
#include "../Core/Engine.h"

void SController::AddComponent(unsigned int entityID, unsigned int &target) {
	components.push_back(CController());
	components.back().Initialize(entityID);
	target = (unsigned int)(components.size() - 1);
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
	
	speedModifier = 2.0;
	sensitivity = 2.0;
}

void CController::MoveForwardBack(double scale) {
	unsigned int transfID = Engine::GetInstance().entities[entityID].components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	trans->position += 5.0f * float(scale * speedModifier) * trans->GetForward();
}

void CController::MoveSide(double scale) {
	unsigned int transfID = engine.entities[entityID].components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	trans->position += 5.0f * float(scale * speedModifier) * trans->GetRight();
}

void CController::MoveVertical(double scale) {
	unsigned int transfID = engine.entities[entityID].components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	trans->position.y += 5.0f * float(scale * speedModifier);
}

void CController::TurnPitch(double scale) {
	unsigned int transfID = engine.entities[entityID].components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];

	trans->angles.x += float(sensitivity * engine.GetUpdateTimeDelta() * scale);

	if (trans->angles.x < -2.4f / 2)	trans->angles.x = -2.4f / 2;
	if (trans->angles.x > 3.14f / 2)	trans->angles.x = 3.14f / 2;
}

void CController::TurnYaw(double scale) {
	unsigned int transfID = engine.entities[entityID].components[COMPONENT_TRANSFORM];
	CTransform *trans = &engine.transformSystem.components[transfID];
	trans->angles.y += float(sensitivity * engine.GetUpdateTimeDelta() * scale);
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
	speedModifier = 4.5;
}

void CController::RunStop(double scale) {
	speedModifier = 2.0;
}
