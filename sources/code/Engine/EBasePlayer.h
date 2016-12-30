#ifndef _EBASE_PLAYER_H
#define _EBASE_PLAYER_H

#include "EBase.h"

class EBasePlayer : public EBase {
public:
	void Spawn();
	void MoveForwardBack(double scale);
	void MoveSide(double scale);
	void MoveVertical(double scale);
	void TurnPitch(double scale);
	void TurnYaw(double scale);
	void SpeedUp(double scale);
	void SpeedDown(double scale);
private:
	double speedModifier;
	double sensitivity;
};

#endif