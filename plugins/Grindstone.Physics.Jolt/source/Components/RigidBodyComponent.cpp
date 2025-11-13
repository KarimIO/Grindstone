#include <Jolt/Jolt.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <Common/HashedString.hpp>

#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>

using namespace JPH::literals;

using namespace Grindstone::Physics;
using namespace Grindstone::Memory;
using namespace Grindstone::Math;

namespace Layers {
	JPH::ObjectLayer MOVING = 0;
}

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
	JPH::Quat rotation(
		transformComponent->rotation.x,
		transformComponent->rotation.y,
		transformComponent->rotation.z,
		transformComponent->rotation.w
	);
	JPH::RVec3 position(
		transformComponent->position.x,
		transformComponent->position.y,
		transformComponent->position.z
	);

	JPH::Shape* shape = colliderComponent->collisionShape;
	JPH::BodyCreationSettings bodySettings(shape, position, rotation, JPH::EMotionType::Dynamic, Layers::MOVING);

	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet.GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		rigidBodyComponent->SetBodyID(
			physWorldContext->GetBodyInterface().CreateAndAddBody(bodySettings, JPH::EActivation::Activate)
		);
	}
}

RigidBodyComponent::RigidBodyComponent(float mass, ColliderComponent* colliderComponent) : mass(mass) {}

RigidBodyComponent RigidBodyComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	RigidBodyComponent newRb;
	newRb.mass = mass;
	newRb.friction = friction;
	newRb.restitution = restitution;
	newRb.dampingLinear = dampingLinear;
	newRb.dampingRotational = dampingRotational;
	newRb.rigidBody = JPH::BodyID();

	return newRb;
}

void RigidBodyComponent::SetFriction(float friction) {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	physWorldContext->GetBodyInterface().SetFriction(rigidBody, friction);
	this->friction = friction;
}

void RigidBodyComponent::SetRestitution(float restitution) {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	physWorldContext->GetBodyInterface().SetRestitution(rigidBody, friction);
	this->restitution = restitution;
}

void RigidBodyComponent::SetDamping(float linear, float rotational) {
}

void RigidBodyComponent::ApplyForce(Float3 pos, Float3 force) {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	physWorldContext->GetBodyInterface().AddForce(rigidBody, JPH::Vec3(force.x, force.y, force.z), JPH::RVec3(pos.x, pos.y, pos.z));
}

void RigidBodyComponent::ApplyCentralForce(Float3 force) {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	physWorldContext->GetBodyInterface().AddForce(rigidBody, JPH::Vec3(force.x, force.y, force.z));
}

void RigidBodyComponent::ApplyImpulse(Float3 pos, Float3 force) {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	physWorldContext->GetBodyInterface().AddImpulse(rigidBody, JPH::Vec3(force.x, force.y, force.z), JPH::RVec3(pos.x, pos.y, pos.z));
}

void RigidBodyComponent::ApplyCentralImpulse(Float3 force) {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	physWorldContext->GetBodyInterface().AddImpulse(rigidBody, JPH::Vec3(force.x, force.y, force.z));
}

float RigidBodyComponent::GetMass() const {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	return physWorldContext->GetBodyInterface().GetShape(rigidBody)->GetMassProperties().mMass;
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

void Grindstone::Physics::RigidBodyComponent::SetBodyID(JPH::BodyID bodyId) {
	rigidBody = bodyId;
}

JPH::BodyID Grindstone::Physics::RigidBodyComponent::GetBodyID() {
	return rigidBody;
}

REFLECT_STRUCT_BEGIN(RigidBodyComponent)
	REFLECT_STRUCT_MEMBER(mass)
	REFLECT_STRUCT_MEMBER(friction)
	REFLECT_STRUCT_MEMBER(restitution)
	REFLECT_STRUCT_MEMBER(dampingLinear)
	REFLECT_STRUCT_MEMBER(dampingRotational)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

// DELETE:
// body_interface.RemoveBody(sphere_id);
// body_interface.DestroyBody(sphere_id);

