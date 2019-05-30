#ifndef _LIGHT_SPOT_SYSTEM_H
#define _LIGHT_SPOT_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include "glm/glm.hpp"

class DepthTarget;
class Framebuffer;

struct LightSpotComponent : public Component {
	LightSpotComponent(GameObjectHandle object_handle, ComponentHandle id);

	struct {
		float attenuationRadius;
		glm::vec3 color;
		float power;
		float innerAngle;
		float outerAngle;
		bool shadow;
		uint16_t resolution;
	} properties_;

	glm::mat4 shadow_mat_;
	Framebuffer *shadow_fbo_;
	DepthTarget *shadow_dt_;
};

class LightSpotSystem : public System {
public:
	LightSpotSystem();
	void update(double dt);
private:
	std::vector<LightSpotComponent> components_;
};

class LightSpotSubSystem : public SubSystem {
	friend LightSpotSystem;
public:
	LightSpotSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	LightSpotComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightSpotSubSystem();
private:
	std::vector<LightSpotComponent> components_;
};

#endif