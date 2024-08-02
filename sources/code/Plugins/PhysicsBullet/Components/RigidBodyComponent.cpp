#include <bullet/btBulletDynamicsCommon.h>

#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Plugins/PhysicsBullet/Core.hpp>

#include "RigidBodyComponent.hpp"

using namespace Grindstone::Physics;
using namespace Grindstone::Memory;
using namespace Grindstone::Math;

static ColliderComponent* GetCollider(entt::registry& registry, entt::entity entityHandle) {
	SphereColliderComponent* sphere = registry.try_get<SphereColliderComponent>(entityHandle);
	if (sphere) {
		return sphere;
	}

	BoxColliderComponent* box = registry.try_get<BoxColliderComponent>(entityHandle);
	if (box) {
		return box;
	}

	PlaneColliderComponent* plane = registry.try_get<PlaneColliderComponent>(entityHandle);
	if (plane) {
		return plane;
	}

	CapsuleColliderComponent* capsule = registry.try_get<CapsuleColliderComponent>(entityHandle);
	if (capsule) {
		return capsule;
	}

	return nullptr;
}

void Grindstone::Physics::SetupRigidBodyComponent(entt::registry& registry, entt::entity entity) {
	ColliderComponent* colliderComponent = GetCollider(registry, entity);
	if (colliderComponent == nullptr) {
		return;
	}

	RigidBodyComponent& rigidBodyComponent = registry.get<RigidBodyComponent>(entity);
	TransformComponent& transformComponent = registry.get<TransformComponent>(entity);

	SetupRigidBodyComponentWithCollider(&rigidBodyComponent, &transformComponent, colliderComponent);
}

void Grindstone::Physics::DestroyRigidBodyComponent(entt::registry& registry, entt::entity entity) {
	RigidBodyComponent& rigidBodyComponent = registry.get<RigidBodyComponent>(entity);
	btRigidBody* rigidbody = rigidBodyComponent.rigidBody;

	if (rigidbody != nullptr) {
		AllocatorCore::Free(rigidbody->getMotionState());
		AllocatorCore::Free(rigidbody);
	}
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

	rigidBodyComponent->rigidBody = AllocatorCore::Allocate<btRigidBody>(
		rigidBodyComponent->GetMass(),
		AllocatorCore::Allocate<btDefaultMotionState>(transformMatrix),
		colliderComponent->collisionShape
	);
	rigidBodyComponent->rigidBody->setUserPointer(&rigidBodyComponent);

	Physics::Core& core = Physics::Core::GetInstance();
	core.dynamicsWorld->addRigidBody(rigidBodyComponent->rigidBody);
}

RigidBodyComponent::RigidBodyComponent(float mass, ColliderComponent* colliderComponent) : mass(mass) {
	btDefaultMotionState* motionState = AllocatorCore::Allocate<btDefaultMotionState>();

	rigidBody = AllocatorCore::Allocate<btRigidBody>(mass, motionState, colliderComponent->collisionShape);
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

float RigidBodyComponent::GetMass() const {
	return mass;
}

float RigidBodyComponent::GetFriction() const {
	return friction;
}

float RigidBodyComponent::GetRestitution() const {
	return restitution;
}

float RigidBodyComponent::GetDampingLinear() const {
	return dampingLinear;
}

float RigidBodyComponent::GetDampingRotational() const {
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
