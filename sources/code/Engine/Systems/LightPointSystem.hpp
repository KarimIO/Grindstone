#ifndef _LIGHT_POINT_SYSTEM_H
#define _LIGHT_POINT_SYSTEM_H

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
	LightPointComponent(ComponentHandle id);
	bool casts_shadow_;
	/*LightPointUBO light_ubu_buffer_;
	UniformBuffer *light_ubo_;
	Framebuffer *shadow_fbo_;
	DepthTarget *shadow_db_;*/
};

class LightPointSystem : public System {
public:
	Component *addComponent();
	void removeComponent(ComponentHandle id);
	void update(double dt);
private:
	std::vector<LightPointComponent> components_;
};

#endif