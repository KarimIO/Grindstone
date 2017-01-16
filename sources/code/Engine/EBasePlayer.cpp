#include "EBasePlayer.h"
#include "Engine.h"

//LINK_ENTITY_TO_CLASS("BasePlayer", EBasePlayer)

void EBasePlayer::Spawn() {
	input.SetInputControlFile("cfgs/player.cfg");
	input.BindAxis("MoveForward", this, &EBasePlayer::MoveForwardBack);
	input.BindAxis("MoveSide", this, &EBasePlayer::MoveSide);
	input.BindAxis("MoveVertical", this, &EBasePlayer::MoveVertical);

	input.BindAxis("TurnPitch", this, &EBasePlayer::TurnPitch);
	input.BindAxis("TurnYaw", this, &EBasePlayer::TurnYaw);

	input.BindAction("ZoomIn", this, &EBasePlayer::ZoomIn);
	input.BindAction("ZoomOut", this, &EBasePlayer::ZoomOut);

	input.BindAction("Run", this, &EBasePlayer::RunStart, KEY_PRESSED);
	input.BindAction("Run", this, &EBasePlayer::RunStop, KEY_RELEASED);
	
	speedModifier = 4;
	sensitivity = 0.2;
}

void EBasePlayer::MoveForwardBack(double scale) {
	position += 5.0f * float(scale * speedModifier) * GetForward();
}

void EBasePlayer::MoveSide(double scale) {
	position += 5.0f * float(scale * speedModifier) * GetRight();
}

void EBasePlayer::MoveVertical(double scale) {
	position.y += 5.0f * float(scale * speedModifier);
}

void EBasePlayer::TurnPitch(double scale) {
	angles.x += float(sensitivity * engine.GetUpdateTimeDelta() * scale);

	if (angles.x < -2.4f / 2)	angles.x = -2.4f / 2;
	if (angles.x > 3.14f / 2)	angles.x = 3.14f / 2;
}

void EBasePlayer::TurnYaw(double scale) {
	angles.y += float(sensitivity * engine.GetUpdateTimeDelta() * scale);
}

void EBasePlayer::ZoomIn(double scale) {
	engine.settings.fov -= 0.05f;
	if (engine.settings.fov < 0.4f)
		engine.settings.fov = 0.4f;
}

void EBasePlayer::ZoomOut(double scale) {
	engine.settings.fov += 0.05f;
	if (engine.settings.fov > 1.57f)
		engine.settings.fov = 1.57f;
}

void EBasePlayer::RunStart(double scale) {

	speedModifier = 8.5;
}

void EBasePlayer::RunStop(double scale) {
	speedModifier = 4.0;
}