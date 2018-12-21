#ifndef _LIGHT_SYSTEM_H
#define _LIGHT_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include "glm/glm.hpp"

struct LightPointUBO {
	glm::vec3 position;
	float attenuationRadius;
	glm::vec3 color;
	float power;
	bool shadow;
};

struct LightPointComponent : public Component {
	LightPointComponent(GameObjectHandle object_handle, ComponentHandle id);
	LightPointUBO properties_;
};

class LightPointSystem : public System {
public:
	LightPointSystem();
	void update(double dt);
private:
	std::vector<LightPointComponent> components_;
};

class LightPointSubSystem : public SubSystem {
	friend LightPointSystem;
public:
	LightPointSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params);
	LightPointComponent &getComponent(ComponentHandle handle);
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightPointSubSystem();
private:
	std::vector<LightPointComponent> components_;
};

#endif