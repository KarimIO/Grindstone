#ifndef _LIGHT_DIRECTIONAL_SYSTEM_H
#define _LIGHT_DIRECTIONAL_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include "glm/glm.hpp"

class DepthTarget;
class Framebuffer;

struct LightDirectionalComponent : public Component {
	LightDirectionalComponent(GameObjectHandle object_handle, ComponentHandle id);

	struct {
		float sourceRadius;
		glm::vec3 color;
		float power;
		bool shadow;
		uint16_t resolution;
	} properties_;

	glm::mat4 shadow_mat_;
	Framebuffer *shadow_fbo_;
	DepthTarget *shadow_dt_;
};

class LightDirectionalSystem : public System {
public:
	LightDirectionalSystem();
	void update(double dt);
private:
	std::vector<LightDirectionalComponent> components_;
};

class LightDirectionalSubSystem : public SubSystem {
	friend LightDirectionalSystem;
public:
	LightDirectionalSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	LightDirectionalComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightDirectionalSubSystem();
private:
	std::vector<LightDirectionalComponent> components_;
};

#endif