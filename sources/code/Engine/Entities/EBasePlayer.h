#ifndef _EBASE_PLAYER_H
#define _EBASE_PLAYER_H

#include "../Core/Input.h"
#include "EBase.h"

class EBasePlayer : public EBase {
public:
	DECLARE_ENTITY();
	void Spawn();
	void MoveForwardBack(double scale);
	void MoveSide(double scale);
	void MoveVertical(double scale);
	void TurnPitch(double scale);
	void TurnYaw(double scale);
	void ZoomIn(double scale);
	void ZoomOut(double scale);
	void RunStart(double scale);
	void RunStop(double scale);
private:
	InputComponent input;
	double speedModifier;
	double sensitivity;
};

#endif