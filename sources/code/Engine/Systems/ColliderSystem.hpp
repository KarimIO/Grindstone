#ifndef _COLLIDER_SYSTEM_H
#define _COLLIDER_SYSTEM_H

#include <vector>
#include "BaseSystem.hpp"

class btCollisionShape;

struct ColliderComponent : public Component {
	// Maybe use union?
	ColliderComponent(GameObjectHandle object_handle, ComponentHandle id);
	enum ShapeType {
		PLANE,
		SPHERE,
		CAPSULE
	} shape_type_;
	btCollisionShape* shape_;
	float sphere_radius_;
	float plane_shape_[4];
	float capsule_radius_;
	float capsule_height_;
};

class ColliderSystem : public System {
public:
	ColliderSystem();
	void update(double dt);
};

class ColliderSubSystem : public SubSystem {
	friend ColliderSystem;
public:
	ColliderSubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	ColliderComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~ColliderSubSystem();
private:
	std::vector<ColliderComponent> components_;
};

#endif