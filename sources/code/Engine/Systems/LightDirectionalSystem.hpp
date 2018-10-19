#ifndef _LIGHT_DIRECTIONAL_SYSTEM_H
#define _LIGHT_DIRECTIONAL_SYSTEM_H

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
	LightDirectionalComponent(ComponentHandle id);
	bool casts_shadow_;
	/*
	LightDirectionalUBO lightUBOBuffer;
	UniformBuffer *lightUBO;
	DepthTarget *shadow_db_;
	std::vector<Framebuffer *>shadow_fbos_;
	std::vector<glm::mat4> matrices_;
	unsigned int res;
	unsigned int cascades_count_;
	*/
};

class LightDirectionalSystem : public System {
public:
	Component *addComponent();
	void removeComponent(ComponentHandle id);
	void update(double dt);
private:
	std::vector<LightDirectionalComponent> components_;
};

#endif