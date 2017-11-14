#ifndef _C_CONTROLLER_H
#define _C_CONTROLLER_H

#include "CBase.h"
#include "Core/Input.h"
#include <vector>

class CController : public CBase {
	double speedModifier;
	double sensitivity;
	InputComponent input;
public:
	void Initialize(unsigned int entityID);
	void MoveForwardBack(double scale);
	void MoveSide(double scale);
	void MoveVertical(double scale);
	void TurnPitch(double scale);
	void TurnYaw(double scale);
	void ZoomIn(double scale);
	void ZoomOut(double scale);
	void RunStart(double scale);
	void RunStop(double scale);
};

class SController {
public:
	void AddComponent(unsigned int entityID, unsigned int &target);
	std::vector<CController> components;
};

#endif