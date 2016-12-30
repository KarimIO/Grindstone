#include "EBasePlayer.h"
#include "Engine.h"

//LINK_ENTITY_TO_CLASS("BasePlayer", EBasePlayer)

void EBasePlayer::Spawn() {
	
	InputComponent input;
	engine.inputSystem.AddControl("s", "MoveForward", &input, -1);
	engine.inputSystem.AddControl("w", "MoveForward", &input, 1);
	engine.inputSystem.AddControl("d", "MoveSide", &input, 1);
	engine.inputSystem.AddControl("a", "MoveSide", &input, -1);
	engine.inputSystem.AddControl("space", "MoveVertical", &input, 1);
	engine.inputSystem.AddControl("control", "MoveVertical", &input, -1);
	engine.inputSystem.AddControl("mousey", "TurnPitch", &input, 1);
	engine.inputSystem.AddControl("mousex", "TurnYaw", &input, 1);
	engine.inputSystem.BindAxis("MoveForward", &input, this, &EBasePlayer::MoveForwardBack);
	engine.inputSystem.BindAxis("MoveSide", &input, this, &EBasePlayer::MoveSide);
	engine.inputSystem.BindAxis("MoveVertical", &input, this, &EBasePlayer::MoveVertical);

	engine.inputSystem.BindAxis("TurnPitch", &input, this, &EBasePlayer::TurnPitch);
	engine.inputSystem.BindAxis("TurnYaw", &input, this, &EBasePlayer::TurnYaw);

	//engine.inputSystem.BindAction("SpeedUp", &input, this, &EBasePlayer::SpeedUp);
	//engine.inputSystem.BindAction("SpeedDown", &input, this, &EBasePlayer::SpeedDown);
	
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
	angles.x += sensitivity * scale;

	if (angles.x < -2.4f / 2)	angles.x = -2.4f / 2;
	if (angles.x > 3.14f / 2)	angles.x = 3.14f / 2;
}

void EBasePlayer::TurnYaw(double scale) {
	angles.y += sensitivity * scale;
}

void EBasePlayer::SpeedUp(double scale) {
	if (speedModifier >= 9.5f)
		speedModifier = 10.0f;
	else
		speedModifier += 0.5f;
}

void EBasePlayer::SpeedDown(double scale) {
	if (speedModifier <= 0.8f)
		speedModifier = 0.6f;
	else
		speedModifier -= 0.2f;
}