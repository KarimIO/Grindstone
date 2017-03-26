#include "Physics.h"
#include <iostream>

#include "Core/Engine.h"

void SPhysics::Initialize() {
	broadphase = new btDbvtBroadphase();

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void CPhysics::SetShapePlane(float Nx, float Ny, float Nz, float c) {
	shape = new btStaticPlaneShape(btVector3(Nx, Ny, Nz), c);
}

void CPhysics::SetShapeSphere(float radius) {
	shape = new btSphereShape(radius);
}

void CPhysics::SetMass(float _mass) {
	mass = _mass;
	// Should we recalculate local inertia here?
}

// Set mass first.
void CPhysics::SetIntertia(float x, float y, float z) {
	btVector3 fallInertia(x, y, z);
	if (mass != 0.0f)
		shape->calculateLocalInertia(mass, fallInertia);
}

void CPhysics::Create() {
	unsigned int transComponentID = engine.entities[entityID].components[COMPONENT_TRANSFORM];
	CTransform *transComponent = &engine.transformSystem.components[transComponentID];
	glm::vec3 posTrans = transComponent->GetPosition();
	glm::vec3 angTrans = transComponent->GetAngles();

	//(glm::orient3(EulerAngles));

	btQuaternion quaternion(angTrans.y, angTrans.x, angTrans.z);
	btVector3 position(posTrans.x, posTrans.y, posTrans.z);

	btTransform trans(quaternion, position);

	btDefaultMotionState* motionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, btVector3(0, 0, 0));
	rigidBody = new btRigidBody(rigidBodyCI);
	engine.physicsSystem.dynamicsWorld->addRigidBody(rigidBody);
}

void CPhysics::Cleanup() {
	engine.physicsSystem.dynamicsWorld->removeRigidBody(rigidBody);
	delete rigidBody->getMotionState();
	delete rigidBody;
	delete shape;
}

void SPhysics::AddComponent(unsigned int entityID, unsigned int &componentID) {
	components.push_back(CPhysics());
	componentID = (unsigned int)(components.size() - 1);
	components[componentID].entityID = entityID;
}

CPhysics *SPhysics::Get(unsigned int componentID) {
	return &components[componentID];
}

CPhysics *SPhysics::GetComponent(unsigned int componentID){
	return &components[componentID];
}

void SPhysics::StepSimulation(double dt) {
	dynamicsWorld->stepSimulation(dt, 10);
}

void SPhysics::SetTransforms() {
	for (unsigned int i = 0; i < (unsigned int)components.size(); i++) {
		btTransform transform;
		components[i].rigidBody->getMotionState()->getWorldTransform(transform);
		btVector3 pos = transform.getOrigin();
		unsigned int entityID = components[i].entityID;
		unsigned int transformID = engine.entities[entityID].components[COMPONENT_TRANSFORM];
		CTransform *transComponent = &engine.transformSystem.components[transformID];
		transComponent->position = glm::vec3(pos.getX(), pos.getY(), pos.getZ());
	}
}

void SPhysics::RemoveComponent(unsigned int id) {
	components[id].Cleanup();
	components.erase(components.begin() + id);
}

void SPhysics::CleanupComponents() {
	for (int i = 0; i < components.size(); i++) {
		components[i].Cleanup();
	}
	components.clear();
}

void SPhysics::Cleanup() {
	CleanupComponents();
	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;
}