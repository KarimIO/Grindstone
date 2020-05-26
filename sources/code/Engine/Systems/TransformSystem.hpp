#ifndef _C_TRANSFORM_H
#define _C_TRANSFORM_H

#include "BaseSystem.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

struct TransformComponent : public Component {
	TransformComponent(GameObjectHandle object_handle, ComponentHandle id);

public:
	void setParent(TransformComponent *tc);
	void addChild(TransformComponent *tc);
	void removeChild(uint32_t i);
	TransformComponent* getChild(uint32_t i);
	void setLocalPosition(glm::vec3 pos);
	void setPosition(glm::vec3 pos);
	void setLocalRotation(glm::quat rot);
	void setRotation(glm::quat rot);
	void setLocalScale(glm::vec3 scale);
	glm::vec3 getLocalPosition();
	glm::vec3 getPosition();
	glm::quat getLocalRotation();
	glm::quat getRotation();
	glm::vec3 getLocalScale();
	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();
	glm::mat4x4& getModelMatrix();

	bool move_with_parent_ = false;
public:
	// Actual Data:
	glm::vec3 local_position_;
	glm::quat local_rotation_;
	glm::vec3 local_scale_;

	// Cached Data:
	glm::vec3 forward_;
	glm::vec3 right_;
	glm::vec3 up_;
	glm::vec3 position_;
	glm::quat rotation_;
	glm::mat4 local_model_;
	glm::mat4 model_;

	// Scene Graph Data:
	TransformComponent* parent_;
	std::vector<TransformComponent *> children_;

	REFLECT(COMPONENT_TRANSFORM)
};

class TransformSystem : public System {
	friend TransformComponent;
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

	virtual ~TransformSubSystem();
private:
	std::vector<TransformComponent> components_;
	std::vector<TransformComponent *> root_components_;
};

#endif