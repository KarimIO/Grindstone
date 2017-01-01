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

	input.BindAction("SpeedUp", this, &EBasePlayer::SpeedUp);
	input.BindAction("SpeedDown", this, &EBasePlayer::SpeedDown);
	
	speedModifier = 1;
	sensitivity = 0.6;
}

void EBasePlayer::MoveForwardBack(double scale) {
	position += 5.0f * float(scale * speedModifier) * getForward();
}

void EBasePlayer::MoveSide(double scale) {
	position += 5.0f * float(scale * speedModifier) * getRight();
}

void EBasePlayer::MoveVertical(double scale) {
	position += 5.0f * float(scale * speedModifier) * getUp();
}

void EBasePlayer::TurnPitch(double scale) {
	angles.x += float(sensitivity * engine.GetTimeDelta() * scale);

	if (angles.x < -2.4f / 2)	angles.x = -2.4f / 2;
	if (angles.x > 3.14f / 2)	angles.x = 3.14f / 2;
}

void EBasePlayer::TurnYaw(double scale) {
	angles.y += float(sensitivity * engine.GetTimeDelta() * scale);
}

void EBasePlayer::SpeedUp(double scale) {
	if (speedModifier >= 2.4f)
		speedModifier = 2.6f;
	else
		speedModifier += 0.2f;
}

void EBasePlayer::SpeedDown(double scale) {
	if (speedModifier <= 0.4f)
		speedModifier = 0.2f;
	else if (speedModifier <= 0.8f)
		speedModifier -= 0.1f;
	else
		speedModifier -= 0.2f;
}