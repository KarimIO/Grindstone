#ifndef _LIGHT_DIRECTIONAL_SYSTEM_H
#define _LIGHT_DIRECTIONAL_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include <array>
#include "glm/glm.hpp"

class Camera;
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
	glm::mat4 view_;
	Framebuffer *shadow_fbo_ = nullptr;
	DepthTarget *shadow_dt_ = nullptr;

	std::vector<std::array<glm::mat4, 4>> camera_matrices_;

	REFLECT()
};

class LightDirectionalSystem : public System {
public:
	LightDirectionalSystem();
	void update();

	void loadGraphics();
	void destroyGraphics();
private:
	std::vector<LightDirectionalComponent> components_;

	REFLECT_SYSTEM()
};

class LightDirectionalSubSystem : public SubSystem {
	friend LightDirectionalSystem;
public:
	void CalcOrthoProjs(Camera &cam, LightDirectionalComponent &comp);
	LightDirectionalSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	void setShadow(ComponentHandle h, bool shadow);
	virtual void initialize() override;
	LightDirectionalComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightDirectionalSubSystem();
private:
	std::vector<LightDirectionalComponent> components_;
};

#endif