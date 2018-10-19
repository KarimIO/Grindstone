#ifndef _LIGHT_SPOT_SYSTEM_H
#define _LIGHT_SPOT_SYSTEM_H

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
	LightSpotComponent(ComponentHandle id);
	bool casts_shadow_;
	/*LightSpotUBO light_ubu_buffer_;
	UniformBuffer *light_ubo_;
	Framebuffer *shadow_fbo_;
	DepthTarget *shadow_db_;*/
};

class LightSpotSystem : public System {
public:
	Component *addComponent();
	void removeComponent(ComponentHandle id);
	void update(double dt);
private:
	std::vector<LightSpotComponent> components_;
};

#endif