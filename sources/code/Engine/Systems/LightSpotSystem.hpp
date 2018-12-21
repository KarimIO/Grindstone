#ifndef _LIGHT_SYSTEM_H
#define _LIGHT_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include "glm/glm.hpp"

struct LightSpotUBO {
	glm::mat4 shadow_mat;
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
	glm::vec3 direction;
	float innerAngle;
	float outerAngle;
	bool shadow;

	char buffer[11];
};

struct LightSpotComponent : public Component {
	LightSpotComponent(GameObjectHandle object_handle, ComponentHandle id);
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
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	LightSpotComponent &getComponent(ComponentHandle handle);
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightSpotSubSystem();
private:
	std::vector<LightSpotComponent> components_;
};

#endif