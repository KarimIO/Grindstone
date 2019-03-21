#ifndef _LIGHT_POINT_SYSTEM_H
#define _LIGHT_POINT_SYSTEM_H

#include "BaseSystem.hpp"
#include <vector>
#include "glm/glm.hpp"

struct LightPointComponent : public Component {
	LightPointComponent(GameObjectHandle object_handle, ComponentHandle id);
	struct {
		glm::vec3 position;
		float attenuationRadius;
		glm::vec3 color;
		float power;
		bool shadow;
	} properties_;
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
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	LightPointComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightPointSubSystem();
private:
	std::vector<LightPointComponent> components_;
};

#endif