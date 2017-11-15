#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "CBase.hpp"
#include <bullet/btBulletDynamicsCommon.h>
#include "glm/glm.hpp"
#include <vector>

class CPhysics : public CBase {
	friend class SPhysics;
private:
	float mass;
	btRigidBody* rigidBody;
	btCollisionShape* shape;
public:
	void SetShapePlane(float Nx, float Ny, float Nz, float c);
	void SetShapeSphere(float radius);

	void SetFriction(float f);
	void SetRestitution(float r);
	void SetDamping(float linear, float rotational);

	void ApplyForce(glm::vec3 pos, glm::vec3 force);
	void ApplyCentralForce(glm::vec3 force);
	void ApplyImpulse(glm::vec3 pos, glm::vec3 force);
	void ApplyCentralImpulse(glm::vec3 force);

	void SetMass(float _mass);
	void SetIntertia(float x, float y, float z);
	void Create();
	void Cleanup();
};

class SPhysics {
	friend class CPhysics;
private:
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	std::vector<CPhysics> components;

	float gravity;
public:
	void Initialize();
	void AddComponent(unsigned int entityID, unsigned int &componentID);
	CPhysics *Get(unsigned int componentID);
	CPhysics *GetComponent(unsigned int componentID);
	void RemoveComponent(unsigned int componentID);
	void StepSimulation(double dt);
	void SetTransforms();
	void CleanupComponents();
	void Cleanup();
};

#endif