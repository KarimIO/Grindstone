#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "CBase.h"
#include <btBulletDynamicsCommon.h>
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