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
	Framebuffer *shadow_fbo_;
	DepthTarget *shadow_dt_;

	std::vector<std::array<glm::mat4, 4>> camera_matrices_;

	REFLECT()
};

class LightDirectionalSystem : public System {
public:
	LightDirectionalSystem();
	void update(double dt);
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
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	void setShadow(ComponentHandle h, bool shadow);
	virtual void initialize() override;
	LightDirectionalComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightDirectionalSubSystem();
private:
	std::vector<LightDirectionalComponent> components_;
};

#endif