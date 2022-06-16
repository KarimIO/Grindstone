#include <bullet/btBulletDynamicsCommon.h>
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Plugins/PhysicsBullet/Core.hpp"
#include "RigidBodyComponent.hpp"
using namespace Grindstone::Physics;
using namespace Grindstone::Math;

ColliderComponent* GetCollider(Grindstone::ECS::Entity& entity) {
	auto& registry = entity.GetSceneEntityRegistry();
	auto entityHandle = entity.GetHandle();
	auto sphere = registry.try_get<SphereColliderComponent>(entityHandle);
	if (sphere) {
		return sphere;
	}

	auto box = registry.try_get<BoxColliderComponent>(entityHandle);
	if (box) {
		return box;
	}

	auto plane = registry.try_get<PlaneColliderComponent>(entityHandle);
	if (plane) {
		return plane;
	}

	auto capsule = registry.try_get<CapsuleColliderComponent>(entityHandle);
	if (capsule) {
		return capsule;
	}

	return nullptr;
}

void Grindstone::Physics::SetupRigidBodyComponent(ECS::Entity& entity, void* componentPtr) {
	auto& registry = entity.GetSceneEntityRegistry();
	auto entityHandle = entity.GetHandle();

	ColliderComponent* colliderComponent = GetCollider(entity);
	if (colliderComponent == nullptr) {
		return;
	}

	RigidBodyComponent* rigidBodyComponent = (RigidBodyComponent*)componentPtr;
	TransformComponent* transformComponent = &registry.get<TransformComponent>(entityHandle);

	SetupRigidBodyComponentWithCollider(rigidBodyComponent, transformComponent, colliderComponent);
}

void Grindstone::Physics::SetupRigidBodyComponentWithCollider(
	RigidBodyComponent* rigidBodyComponent,
	TransformComponent* transformComponent,
	ColliderComponent* colliderComponent
) {
	btScalar mass = rigidBodyComponent->GetMass();
	btQuaternion quaternion;
	btVector3 position(
		transformComponent->position.x,
		transformComponent->position.y,
		transformComponent->position.z
	);
	btTransform transformMatrix(quaternion, position);

	rigidBodyComponent->rigidBody = new btRigidBody(
		rigidBodyComponent->GetMass(),
		new btDefaultMotionState(transformMatrix),
		colliderComponent->collisionShape
	);
	rigidBodyComponent->rigidBody->setUserPointer(&rigidBodyComponent);

	auto& core = Physics::Core::GetInstance();
	core.dynamicsWorld->addRigidBody(rigidBodyComponent->rigidBody);
}

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

float RigidBodyComponent::GetMass() {
	return mass;
}

float RigidBodyComponent::GetFriction() {
	return friction;
}

float RigidBodyComponent::GetRestitution() {
	return restitution;
}

float RigidBodyComponent::GetDampingLinear() {
	return dampingLinear;
}

float RigidBodyComponent::GetDampingRotational() {
	return dampingRotational;
}

REFLECT_STRUCT_BEGIN(RigidBodyComponent)
	REFLECT_STRUCT_MEMBER(mass)
	REFLECT_STRUCT_MEMBER(friction)
	REFLECT_STRUCT_MEMBER(restitution)
	REFLECT_STRUCT_MEMBER(dampingLinear)
	REFLECT_STRUCT_MEMBER(dampingRotational)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
