#ifndef _CAMERA_SYSTEM_H
#define _CAMERA_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"
#include <glm/glm.hpp>
#include <PostProcess/PostPipeline.hpp>
#include "../Core/Camera.hpp"

struct CameraComponent : public Component {
	CameraComponent(Space *space, GameObjectHandle object_handle, ComponentHandle id);

	Camera camera_;
};

class CameraSystem : public System {
public:
	CameraSystem();

	void update(double dt);
private:
};

class CameraSubSystem : public SubSystem {
	friend CameraSystem;
public:
	CameraSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	CameraComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	virtual ~CameraSubSystem();
private:
	std::vector<CameraComponent> components_;
};

#endif