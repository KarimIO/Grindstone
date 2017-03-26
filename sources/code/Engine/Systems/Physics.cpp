#include "Physics.h"
#include <iostream>

#include "Core/Engine.h"

void SPhysics::Initialize() {
	broadphase = new btDbvtBroadphase();

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -9.81, 0));
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

void CPhysics::SetFriction(float f) {
	rigidBody->setFriction(f);
}

void CPhysics::SetRestitution(float r) {
	rigidBody->setRestitution(r);
}

void CPhysics::SetDamping(float linear, float rotational) {
	rigidBody->setDamping(linear, rotational);
}

void CPhysics::ApplyForce(glm::vec3 pos, glm::vec3 force) {
	rigidBody->applyForce(btVector3(pos.x(), pos.y(), pos.z()), btVector3(force.x(), force.y(), force.z()));
}

void CPhysics::ApplyCentralForce(glm::vec3 force) {
	rigidBody->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
}

void CPhysics::ApplyImpulse(glm::vec3 pos, glm::vec3 force) {
	rigidBody->applyForce(btVector3(pos.x(), pos.y(), pos.z()), btVector3(force.x(), force.y(), force.z()));
}

void CPhysics::ApplyCentralImpulse(glm::vec3 force) {
	rigidBody->applyCentralForce(btVector3(force.x(), force.y(), force.z()));
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
		btQuaternion q = transform.getRotation();
		std::cout << q.x() << " " << q.y() << " " << q.z() << " " << q.w() << "\n";

		double ysqr = q.y() * q.y();

		// roll (x-axis rotation)
		double t0 = +2.0 * (q.w() * q.x() + q.y() * q.z());
		double t1 = +1.0 - 2.0 * (q.x() * q.x() + ysqr);
		transComponent->angles.z = std::atan2(t0, t1);

		// pitch (y-axis rotation)
		double t2 = +2.0 * (q.w() * q.y() - q.z() * q.x());
		t2 = t2 > 1.0 ? 1.0 : t2;
		t2 = t2 < -1.0 ? -1.0 : t2;
		transComponent->angles.x = std::asin(t2);

		// yaw (z-axis rotation)
		double t3 = +2.0 * (q.w() * q.z() + q.x() * q.y());
		double t4 = +1.0 - 2.0 * (ysqr + q.z() * q.z());
		transComponent->angles.y = std::atan2(t3, t4);
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