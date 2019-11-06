#ifndef _CONTROLLER_SYSTEM_H
#define _CONTROLLER_SYSTEM_H

#include "BaseSystem.hpp"
#include "Core/Input.hpp"
#include <vector>
#include <glm/glm.hpp>

class TransformSubSystem;

class ControllerComponent : public Component {
public:
	ControllerComponent(GameObjectHandle object_handle, ComponentHandle handle);
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
	TransformSubSystem *getTransform();
private:
	bool ghost_mode_;
	bool no_collide_;
	double speed_modifier_;
	double sensitivity_;
	glm::vec3 velocity_;
	InputComponent	input;

	REFLECT()
};

class ControllerSystem : public System {
public:
	ControllerSystem();
	void update();
private:
	UniformBuffer *ubo_;

	REFLECT_SYSTEM()
};

class ControllerSubSystem : public SubSystem {
	friend ControllerSystem;
public:
	ControllerSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	ControllerComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~ControllerSubSystem();
	size_t getNumComponents();
private:
	std::vector<ControllerComponent> components_;
};

#endif