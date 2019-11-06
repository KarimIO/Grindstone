#ifndef _C_TRANSFORM_H
#define _C_TRANSFORM_H

#include "BaseSystem.hpp"
#include <glm/glm.hpp>
#include <vector>

struct TransformComponent : public Component {
	TransformComponent(GameObjectHandle object_handle, ComponentHandle id);

	glm::vec3 position_;
	glm::vec3 angles_;
	glm::vec3 scale_;
	glm::mat4 model_;

	 REFLECT()
};

class TransformSystem : public System {
public:
	TransformSystem();
	void update();
private:

	 REFLECT_SYSTEM()
};

class TransformSubSystem : public SubSystem {
	friend TransformSystem;
public:
	TransformSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	TransformComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);

	glm::vec3 getForward(ComponentHandle handle);
	glm::vec3 getRight(ComponentHandle handle);
	glm::vec3 getUp(ComponentHandle handle);
	glm::vec3 getAngles(ComponentHandle handle);
	glm::vec3 getPosition(ComponentHandle handle);
	glm::vec3 getScale(ComponentHandle handle);
	glm::mat4x4 &getModelMatrix(ComponentHandle handle);

	virtual ~TransformSubSystem();
private:
	std::vector<TransformComponent> components_;
};

#endif