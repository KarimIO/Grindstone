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

	REFLECT()
};

class LightPointSystem : public System {
public:
	LightPointSystem();
	void update(double dt);

	void loadGraphics();
	void destroyGraphics();
private:
	std::vector<LightPointComponent> components_;

	REFLECT_SYSTEM()
};

class LightPointSubSystem : public SubSystem {
	friend LightPointSystem;
public:
	LightPointSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	void setShadow(ComponentHandle h, bool shadow);
	virtual void initialize() override;
	LightPointComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);

	virtual ~LightPointSubSystem();
private:
	std::vector<LightPointComponent> components_;
};

#endif