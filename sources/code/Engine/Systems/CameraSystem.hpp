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
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	CameraComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~CameraSubSystem();
private:
	std::vector<CameraComponent> components_;
};

#endif