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

static JPH::BodyInterface& GetActiveBodyInterface() {
	Grindstone::WorldContextManager* mgr = Grindstone::EngineCore::GetInstance().GetWorldContextManager();
	Grindstone::WorldContextSet* cxtSet = mgr->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));

	return physWorldContext->GetBodyInterface();
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
	JPH::Quat rotation = JPH::Quat(
		transformComponent->rotation.x,
		transformComponent->rotation.y,
		transformComponent->rotation.z,
		transformComponent->rotation.w
	).Normalized();
	JPH::RVec3 position(
		transformComponent->position.x,
		transformComponent->position.y,
		transformComponent->position.z
	);

	JPH::EMotionType motionType = rigidBodyComponent->isStatic
		? JPH::EMotionType::Static
		: JPH::EMotionType::Dynamic;

	JPH::Shape* shape = colliderComponent->collisionShape;
	JPH::BodyCreationSettings bodySettings(shape, position, rotation, motionType, rigidBodyComponent->layer.AsUint8());

	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet.GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		rigidBodyComponent->SetBodyID(
			physWorldContext->GetBodyInterface().CreateAndAddBody(bodySettings, JPH::EActivation::Activate)
		);
	}
}

RigidBodyComponent::RigidBodyComponent(float mass, ColliderComponent* colliderComponent) : mass(mass) {}

RigidBodyComponent::~RigidBodyComponent() {
	// TODO: Maybe we need to pass cxt set here? That would mean a destructor that passes contexts.
	Grindstone::WorldContextSet* cxtSet = Grindstone::EngineCore::GetInstance().GetWorldContextManager()->GetActiveWorldContextSet();
	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet->GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		physWorldContext->GetBodyInterface().RemoveBody(GetBodyID());
	}
}

RigidBodyComponent RigidBodyComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	RigidBodyComponent newRb;
	newRb.isStatic = isStatic;
	newRb.layer = layer;
	newRb.mass = mass;
	newRb.friction = friction;
	newRb.restitution = restitution;
	newRb.rigidBody = JPH::BodyID();

	return newRb;
}

void RigidBodyComponent::SetIsStatic(bool isStatic) {
	JPH::EMotionType motionType = isStatic
		? JPH::EMotionType::Static
		: JPH::EMotionType::Dynamic;

	JPH::EActivation activationMode = JPH::EActivation::Activate;

	GetActiveBodyInterface().SetMotionType(rigidBody, motionType, activationMode);
	this->isStatic = isStatic;
}

void RigidBodyComponent::SetLayer(Grindstone::Physics::Layer layer) {
	GetActiveBodyInterface().SetObjectLayer(rigidBody, layer.AsUint8());
	this->friction = friction;
}

void RigidBodyComponent::SetFriction(float friction) {
	GetActiveBodyInterface().SetFriction(rigidBody, friction);
	this->friction = friction;
}

void RigidBodyComponent::SetRestitution(float restitution) {
	GetActiveBodyInterface().SetRestitution(rigidBody, friction);
	this->restitution = restitution;
}

void RigidBodyComponent::SetPosition(Math::Float3 pos) {
	GetActiveBodyInterface().SetPosition(rigidBody, JPH::Vec3(pos.x, pos.y, pos.z), JPH::EActivation::Activate);
}

void RigidBodyComponent::SetRotation(Math::Quaternion rot) {
	GetActiveBodyInterface().SetRotation(rigidBody, JPH::Quat(rot.w, rot.x, rot.y, rot.z), JPH::EActivation::Activate);
}

void RigidBodyComponent::ApplyForce(Float3 pos, Float3 force) {
	GetActiveBodyInterface().AddForce(rigidBody, JPH::Vec3(force.x, force.y, force.z), JPH::RVec3(pos.x, pos.y, pos.z));
}

void RigidBodyComponent::ApplyCentralForce(Float3 force) {
	GetActiveBodyInterface().AddForce(rigidBody, JPH::Vec3(force.x, force.y, force.z));
}

void RigidBodyComponent::ApplyImpulse(Float3 pos, Float3 force) {
	GetActiveBodyInterface().AddImpulse(rigidBody, JPH::Vec3(force.x, force.y, force.z), JPH::RVec3(pos.x, pos.y, pos.z));
}

void RigidBodyComponent::ApplyCentralImpulse(Float3 force) {
	GetActiveBodyInterface().AddImpulse(rigidBody, JPH::Vec3(force.x, force.y, force.z));
}

bool RigidBodyComponent::GetIsStatic() const {
	return GetActiveBodyInterface().GetMotionType(rigidBody) == JPH::EMotionType::Static;
}

Grindstone::Physics::Layer RigidBodyComponent::GetLayer() const {
	return GetActiveBodyInterface().GetObjectLayer(rigidBody);
}

float RigidBodyComponent::GetMass() const {
	return GetActiveBodyInterface().GetShape(rigidBody)->GetMassProperties().mMass;
}

float RigidBodyComponent::GetFriction() const {
	return friction;
}

float RigidBodyComponent::GetRestitution() const {
	return restitution;
}

Grindstone::Math::Float3 RigidBodyComponent::GetPosition() const {
	JPH::Vec3 p = GetActiveBodyInterface().GetPosition(rigidBody);
	return Grindstone::Math::Float3(p.GetX(), p.GetY(), p.GetZ());
}

Grindstone::Math::Quaternion RigidBodyComponent::GetRotation() const {
	JPH::Quat q = GetActiveBodyInterface().GetRotation(rigidBody);
	return Grindstone::Math::Quaternion(q.GetY(), q.GetZ(), q.GetW(), q.GetX());
}

void Grindstone::Physics::RigidBodyComponent::SetBodyID(JPH::BodyID bodyId) {
	rigidBody = bodyId;
}

JPH::BodyID Grindstone::Physics::RigidBodyComponent::GetBodyID() const {
	return rigidBody;
}

REFLECT_STRUCT_BEGIN(RigidBodyComponent)
	REFLECT_STRUCT_MEMBER(isStatic)
	REFLECT_STRUCT_MEMBER(layer)
	REFLECT_STRUCT_MEMBER(mass)
	REFLECT_STRUCT_MEMBER(friction)
	REFLECT_STRUCT_MEMBER(restitution)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

extern "C" {
	JOLT_PHYSICS_EXPORT void* EntityGetRigidbodyComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = Grindstone::EngineCore::GetInstance().GetEntityRegistry();
		const entt::entity entityId = static_cast<entt::entity>(entity);
		Grindstone::Physics::RigidBodyComponent* comp = reg.try_get<Grindstone::Physics::RigidBodyComponent>(entityId);
		return comp;
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentApplyForce(Grindstone::Physics::RigidBodyComponent& comp, Grindstone::Math::ExportableVector pos, Grindstone::Math::ExportableVector force) {
		comp.ApplyForce(Grindstone::Math::ImportVector(pos), Grindstone::Math::ImportVector(force));
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentApplyCentralForce(Grindstone::Physics::RigidBodyComponent& comp, Grindstone::Math::ExportableVector force) {
		comp.ApplyCentralForce(Grindstone::Math::ImportVector(force));
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentApplyImpulse(Grindstone::Physics::RigidBodyComponent& comp, Grindstone::Math::ExportableVector pos, Grindstone::Math::ExportableVector force) {
		comp.ApplyImpulse(Grindstone::Math::ImportVector(pos), Grindstone::Math::ImportVector(force));
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentApplyCentralImpulse(Grindstone::Physics::RigidBodyComponent& comp, Grindstone::Math::ExportableVector force) {
		comp.ApplyCentralImpulse(Grindstone::Math::ImportVector(force));
	}

	JOLT_PHYSICS_EXPORT bool RigidbodyComponentGetIsStatic(Grindstone::Physics::RigidBodyComponent& comp) {
		return comp.GetIsStatic();
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentSetIsStatic(Grindstone::Physics::RigidBodyComponent& comp, bool isStatic) {
		return comp.SetIsStatic(isStatic);
	}

	JOLT_PHYSICS_EXPORT int RigidbodyComponentGetLayer(Grindstone::Physics::RigidBodyComponent& comp) {
		return static_cast<uint32_t>(comp.GetLayer().AsUint8());
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentSetLayer(Grindstone::Physics::RigidBodyComponent& comp, int layer) {
		comp.SetLayer(Grindstone::Physics::Layer(static_cast<uint8_t>(layer)));
	}

	JOLT_PHYSICS_EXPORT float RigidbodyComponentGetMass(Grindstone::Physics::RigidBodyComponent& comp) {
		return comp.GetMass();
	}

	JOLT_PHYSICS_EXPORT float RigidbodyComponentGetFriction(Grindstone::Physics::RigidBodyComponent& comp) {
		return comp.GetFriction();
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentSetFriction(Grindstone::Physics::RigidBodyComponent& comp, float friction) {
		return comp.SetFriction(friction);
	}

	JOLT_PHYSICS_EXPORT float RigidbodyComponentGetRestitution(Grindstone::Physics::RigidBodyComponent& comp) {
		return comp.GetRestitution();
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentSetRestitution(Grindstone::Physics::RigidBodyComponent& comp, float restitution) {
		return comp.SetRestitution(restitution);
	}

	JOLT_PHYSICS_EXPORT Grindstone::Math::ExportableVector RigidbodyComponentGetPosition(Grindstone::Physics::RigidBodyComponent& component) {
		return Grindstone::Math::ExportVector(component.GetPosition());
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentSetPosition(Grindstone::Physics::RigidBodyComponent& component, Grindstone::Math::ExportableVector position) {
		component.SetPosition(Grindstone::Math::ImportVector(position));
	}

	JOLT_PHYSICS_EXPORT Grindstone::Math::ExportableQuaternion RigidbodyComponentGetRotation(Grindstone::Physics::RigidBodyComponent& component) {
		return Grindstone::Math::ExportQuaternion(component.GetRotation());
	}

	JOLT_PHYSICS_EXPORT void RigidbodyComponentSetRotation(Grindstone::Physics::RigidBodyComponent& component, Grindstone::Math::ExportableQuaternion rotation) {
		component.SetRotation(Grindstone::Math::ImportQuaternion(rotation));
	}
}
