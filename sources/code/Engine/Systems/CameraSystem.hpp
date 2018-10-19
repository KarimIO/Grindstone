#ifndef _CAMERA_SYSTEM_H
#define _CAMERA_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "BaseSystem.hpp"

struct CameraComponent : public Component {
	CameraComponent(ComponentHandle id);

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

	Component *addComponent();
	void removeComponent(ComponentHandle id);

	void update(double dt);
private:
	std::vector<CameraComponent> components_;
};

#endif