#ifndef _LIGHT_SYSTEM_H
#define _LIGHT_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include "glm/glm.hpp"

struct LightDirectionalUBO {
	glm::mat4 shadow_mat[3];
	float cascade_distance[3];
	glm::vec3 direction;
	float sourceRadius;
	glm::vec3 color;
	float power;
	bool shadow;
};

struct LightDirectionalComponent : public Component {
	LightDirectionalComponent(GameObjectHandle object_handle, ComponentHandle id);
	size_t getNumComponents();
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
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightDirectionalSubSystem();
private:
	std::vector<LightDirectionalComponent> components_;
};

#endif