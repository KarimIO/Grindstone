#include <bullet/btBulletDynamicsCommon.h>
#include "RigidBodyComponent.hpp"
using namespace Grindstone::Physics;
using namespace Grindstone::Math;

RigidBodyComponent::RigidBodyComponent(float mass, ColliderComponent* colliderComponent) : mass(mass) {
	btDefaultMotionState* motionState = new btDefaultMotionState();

	rigidBody = new btRigidBody(mass, motionState, colliderComponent->collisionShape);
}

void RigidBodyComponent::SetCollisionShape(ColliderComponent* colliderComponent) {
	rigidBody->setCollisionShape(colliderComponent->collisionShape);
}

void RigidBodyComponent::SetFriction(float friction) {
	rigidBody->setFriction(friction);
	this->friction = friction;
}

void RigidBodyComponent::SetRestitution(float restitution) {
	rigidBody->setRestitution(restitution);
	this->restitution = restitution;
}

void RigidBodyComponent::SetDamping(float linear, float rotational) {
	rigidBody->setDamping(linear, rotational);
	this->dampingLinear = linear;
	this->dampingRotational = rotational;
}

void RigidBodyComponent::ApplyForce(Float3 pos, Float3 force) {
	rigidBody->applyForce(btVector3(pos.x, pos.y, pos.z), btVector3(force.x, force.y, force.z));
}

void RigidBodyComponent::ApplyCentralForce(Float3 force) {
	rigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
}

void RigidBodyComponent::ApplyImpulse(Float3 pos, Float3 force) {
	rigidBody->applyForce(btVector3(pos.x, pos.y, pos.z), btVector3(force.x, force.y, force.z));
}

void RigidBodyComponent::ApplyCentralImpulse(Float3 force) {
	rigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
}

REFLECT_STRUCT_BEGIN(RigidBodyComponent)
	REFLECT_STRUCT_MEMBER(mass)
	REFLECT_STRUCT_MEMBER(friction)
	REFLECT_STRUCT_MEMBER(restitution)
	REFLECT_STRUCT_MEMBER(dampingLinear)
	REFLECT_STRUCT_MEMBER(dampingRotational)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
