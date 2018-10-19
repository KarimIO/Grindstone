#ifndef _C_TRANSFORM_H
#define _C_TRANSFORM_H

#include "BaseSystem.hpp"
#include <glm/glm.hpp>
#include <vector>

struct TransformComponent : public Component {
	TransformComponent(ComponentHandle id);

	ComponentHandle parent_;
	glm::vec3 position_;
	glm::vec3 velocity_;
	glm::vec3 angles_;
	glm::vec3 scale_;
	glm::mat4 model_;
};

class TransformSystem : public System {
public:
	Component *addComponent();
	void removeComponent(ComponentHandle id);

	void update(double dt);
	glm::vec3 getForward(ComponentHandle handle);
	glm::vec3 getRight(ComponentHandle handle);
	glm::vec3 getUp(ComponentHandle handle);
	glm::vec3 getAngles(ComponentHandle handle);
	glm::vec3 getPosition(ComponentHandle handle);
	glm::vec3 getVelocity(ComponentHandle handle);
	glm::vec3 getScale(ComponentHandle handle);
	glm::mat4x4 &getModelMatrix(ComponentHandle handle);
private:
	std::vector<TransformComponent> components_;
	inline TransformComponent &getComponent(ComponentHandle handle);
};

#endif