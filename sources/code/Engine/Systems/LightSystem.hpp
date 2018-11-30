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

struct LightDirectionalUBO {
	glm::mat4 shadow_mat[3];
	float cascade_distance[3];
	glm::vec3 direction;
	float sourceRadius;
	glm::vec3 color;
	float power;
	bool shadow;
};

struct LightPointUBO {
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
	bool shadow;
};

struct LightComponent : public Component {
	LightComponent(GameObjectHandle object_handle, ComponentHandle id);
};

class LightSystem : public System {
public:
	LightSystem();
	void update(double dt);
private:
	std::vector<LightComponent> components_;
};

class LightSubSystem : public SubSystem {
	friend LightSystem;
public:
	LightSubSystem();
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	LightComponent &getComponent(ComponentHandle handle);
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightSubSystem();
private:
	std::vector<LightComponent> components_;
};

#endif