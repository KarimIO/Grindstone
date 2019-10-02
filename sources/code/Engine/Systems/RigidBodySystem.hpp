#ifndef _PHYSICS_SYSTEM_H
#define _PHYSICS_SYSTEM_H

#include "BaseSystem.hpp"
#include "glm/glm.hpp"
#include <vector>

class btRigidBody;
class btCollisionShape;
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

class RigidBodyComponent : public Component {
	friend class SPhysics;
public:
	RigidBodyComponent(GameObjectHandle object_handle, ComponentHandle id);

	void setMass(float mass);
	void setIntertia(float x, float y, float z);
	void setFriction(float f);
	void setRestitution(float r);
	void setDamping(float linear, float rotational);

	void applyForce(glm::vec3 pos, glm::vec3 force);
	void applyCentralForce(glm::vec3 force);
	void applyImpulse(glm::vec3 pos, glm::vec3 force);
	void applyCentralImpulse(glm::vec3 force);

	float mass_;
	float friction_;
	float restitution_;
	float damping_linear_;
	float damping_rotational_;
	btRigidBody* rigid_body_;
	btCollisionShape* shape_;
};

class RigidBodySystem : public System {
public:
	RigidBodySystem();
	void update(double dt);
};

class RigidBodySubSystem : public SubSystem {
	friend RigidBodySystem;
public:
	RigidBodySubSystem(Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	virtual void setComponent(ComponentHandle component_handle, rapidjson::Value & params) override;
	RigidBodyComponent &getComponent(ComponentHandle handle);
	virtual Component *getBaseComponent(ComponentHandle component_handle) override;
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RigidBodySubSystem();
//private:
	btBroadphaseInterface* broadphase_;
	btDefaultCollisionConfiguration* collision_configuration_;
	btCollisionDispatcher* dispatcher_;
	btSequentialImpulseConstraintSolver* solver_;
	btDiscreteDynamicsWorld* dynamics_world_;

	std::vector<RigidBodyComponent> components_;
};

#endif