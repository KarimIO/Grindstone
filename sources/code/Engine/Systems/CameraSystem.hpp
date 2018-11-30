#ifndef _CAMERA_SYSTEM_H
#define _CAMERA_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "BaseSystem.hpp"

class RenderPath;

struct CameraComponent : public Component {
	CameraComponent(GameObjectHandle object_handle, ComponentHandle id);

	bool is_ortho;
	double ortho_x_;
	double ortho_y_;
	double ortho_width_;
	double ortho_height_;

	double projection_fov_;

	double viewport_x_;
	double viewport_y_;
	double viewport_width_;
	double viewport_height_;
	double near_;
	double far_;
	double aperture_size_;
	double shutter_speed_;
	double iso_;
	glm::mat4 projection_;
	glm::mat4 view_;
};

class CameraSystem : public System {
public:
	CameraSystem();

	RenderPath *render_path_;

	void update(double dt);
private:
	UniformBuffer *ubo_;
};

class CameraSubSystem : public SubSystem {
	friend CameraSystem;
public:
	CameraSubSystem();
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	CameraComponent &getComponent(ComponentHandle handle);
	virtual void removeComponent(ComponentHandle handle);
	virtual ~CameraSubSystem();
private:
	std::vector<CameraComponent> components_;
};

#endif