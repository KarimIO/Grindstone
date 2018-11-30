#if 0
#ifndef _C_CONTROLLER_H
#define _C_CONTROLLER_H

#include "CBase.hpp"
#include "Core/Input.hpp"
#include <vector>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

class CController : public CBase {
private:
	bool ghost_mode_;
	bool no_collide_;
	double speed_modifier_;
	double sensitivity_;
	InputComponent	input;
	btCollisionShape	*shape;
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
	void update(double dt);
};

class SController {
public:
	void AddComponent(unsigned int entityID, unsigned int &target);
	void update(double dt);
	std::vector<CController> components;
};

#endif
#endif