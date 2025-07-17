#include <bullet/btBulletDynamicsCommon.h>

#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <Plugins/PhysicsBullet/PhysicsWorldContext.hpp>
#include <Common/HashedString.hpp>

#include "RigidBodyComponent.hpp"

using namespace Grindstone::Physics;
using namespace Grindstone::Memory;
using namespace Grindstone::Math;

static ColliderComponent* GetCollider(entt::registry& registry, entt::entity entityHandle) {
	SphereColliderComponent* sphere = registry.try_get<SphereColliderComponent>(entityHandle);
	if (sphere != nullptr) {
		return sphere;
	}

	BoxColliderComponent* box = registry.try_get<BoxColliderComponent>(entityHandle);
	if (box != nullptr) {
		return box;
	}

	PlaneColliderComponent* plane = registry.try_get<PlaneColliderComponent>(entityHandle);
	if (plane != nullptr) {
		return plane;
	}

	CapsuleColliderComponent* capsule = registry.try_get<CapsuleColliderComponent>(entityHandle);
	if (capsule != nullptr) {
		return capsule;
	}

	return nullptr;
}

void Grindstone::Physics::SetupRigidBodyComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	entt::registry& registry = cxtSet.GetEntityRegistry();
	ColliderComponent* colliderComponent = GetCollider(registry, entity);
	if (colliderComponent == nullptr) {
		return;
	}

	RigidBodyComponent& rigidBodyComponent = registry.get<RigidBodyComponent>(entity);
	TransformComponent& transformComponent = registry.get<TransformComponent>(entity);

	SetupRigidBodyComponentWithCollider(cxtSet, &rigidBodyComponent, &transformComponent, colliderComponent);
}

void Grindstone::Physics::SetupRigidBodyComponentWithCollider(
	Grindstone::WorldContextSet& cxtSet,
	RigidBodyComponent* rigidBodyComponent,
	TransformComponent* transformComponent,
	ColliderComponent* colliderComponent
) {
	btScalar mass = rigidBodyComponent->GetMass();
	btQuaternion quaternion(
		transformComponent->rotation.x,
		transformComponent->rotation.y,
		transformComponent->rotation.z,
		transformComponent->rotation.w
	);
	btVector3 position(
		transformComponent->position.x,
		transformComponent->position.y,
		transformComponent->position.z
	);
	btTransform transformMatrix(quaternion, position);

	rigidBodyComponent->motionState = AllocatorCore::AllocateUnique<btDefaultMotionState>(transformMatrix);
	rigidBodyComponent->rigidBody = AllocatorCore::AllocateUnique<btRigidBody>(
		rigidBodyComponent->GetMass(),
		rigidBodyComponent->motionState.Get(),
		colliderComponent->collisionShape.Get()
	);
	rigidBodyComponent->rigidBody->setUserPointer(&rigidBodyComponent);

	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet.GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		physWorldContext->dynamicsWorld->addRigidBody(rigidBodyComponent->rigidBody.Get());
	}
}

RigidBodyComponent::RigidBodyComponent(float mass, ColliderComponent* colliderComponent) : mass(mass) {
	motionState = AllocatorCore::AllocateUnique<btDefaultMotionState>();

	rigidBody = AllocatorCore::AllocateUnique<btRigidBody>(mass, motionState.Get(), colliderComponent->collisionShape.Get());
	rigidBody->setUserPointer(this);
}

RigidBodyComponent RigidBodyComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	RigidBodyComponent newRb;
	newRb.mass = mass;
	newRb.friction = friction;
	newRb.restitution = restitution;
	newRb.dampingLinear = dampingLinear;
	newRb.dampingRotational = dampingRotational;
	newRb.rigidBody = nullptr;

	return newRb;
}

void RigidBodyComponent::SetCollisionShape(ColliderComponent* colliderComponent) {
	rigidBody->setCollisionShape(colliderComponent->collisionShape.Get());
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
