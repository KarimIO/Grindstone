#ifndef _COLLIDER_SYSTEM_H
#define _COLLIDER_SYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "BaseSystem.hpp"

class btCollisionShape;

struct ColliderComponent : public Component {
	// Maybe use union?
	ColliderComponent(GameObjectHandle object_handle, ComponentHandle id);
	/*enum class ShapeType {
		PLANE,
		SPHERE,
		BOX,
		CAPSULE
	} */
	int shape_type_;
	btCollisionShape* shape_;
	float sphere_radius_;
	glm::vec4 plane_shape_;
	glm::vec3 box_offset_;
	glm::vec3 box_size_;
	float capsule_radius_;
	float capsule_height_;

	REFLECT()
};

class ColliderSystem : public System {
public:
	ColliderSystem();
	void update();

	REFLECT_SYSTEM()
};

class ColliderSubSystem : public SubSystem {
	friend ColliderSystem;
public:
	ColliderSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	ColliderComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void removeComponent(ComponentHandle handle);
	virtual void initialize() override;
	virtual ~ColliderSubSystem();
private:
	std::vector<ColliderComponent> components_;
};

#endif