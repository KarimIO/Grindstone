#include <Grindstone.Physics.Jolt/include/pch.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Character/Character.h>

#include <Common/Math.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Physics.Jolt/include/Components/CharacterRigidbodyControllerComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

using namespace JPH::literals;

using namespace Grindstone::Physics;
using namespace Grindstone::Math;

REFLECT_STRUCT_BEGIN(CharacterRigidbodyControllerComponent)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::Physics::SetupCharacterRigidbodyControllerComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	entt::registry& registry = cxtSet.GetEntityRegistry();
	ColliderComponent* colliderComponent = GetCollider(registry, entity);
	if (colliderComponent == nullptr || colliderComponent->collisionShape == nullptr) {
		return;
	}

	CharacterRigidbodyControllerComponent& CharacterRigidbodyControllerComponent = registry.get<Physics::CharacterRigidbodyControllerComponent>(entity);
	TransformComponent& transformComponent = registry.get<TransformComponent>(entity);

	SetupCharacterRigidbodyControllerComponentWithCollider(cxtSet, &CharacterRigidbodyControllerComponent, &transformComponent, colliderComponent);
}

void Grindstone::Physics::SetupCharacterRigidbodyControllerComponentWithCollider(
	Grindstone::WorldContextSet& cxtSet,
	CharacterRigidbodyControllerComponent* characterComponent,
	TransformComponent* transformComponent,
	ColliderComponent* colliderComponent
) {
	JPH::Quat rotation = JPH::Quat(
		transformComponent->rotation.x,
		transformComponent->rotation.y,
		transformComponent->rotation.z,
		transformComponent->rotation.w
	).Normalized();

	JPH::Vec3 position(
		transformComponent->position.x,
		transformComponent->position.y,
		transformComponent->position.z
	);

	JPH::Shape* shape = colliderComponent->collisionShape;
	
	JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings();
	settings->mShape = shape;
	settings->mLayer = Layers::MOVING;
	settings->mMass = 80.0f;
	settings->mGravityFactor = 1.0f;
	settings->mFriction = 10.0f;

	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet.GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		JPH::uint64 userParameter = 0;
		JPH::PhysicsSystem* physSystem = &physWorldContext->GetPhysicsSystem();
		characterComponent->character = new JPH::Character(settings, position, rotation, userParameter, physSystem);
		characterComponent->character->AddToPhysicsSystem(JPH::EActivation::Activate);
	}
}

CharacterRigidbodyControllerComponent CharacterRigidbodyControllerComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	CharacterRigidbodyControllerComponent character{};

	return character;
}

bool CharacterRigidbodyControllerComponent::IsOnGround() const {
	return (character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround);
}

void CharacterRigidbodyControllerComponent::SetVelocity(Math::Float3 velocity) {
	character->SetLinearVelocity(JPH::Vec3(velocity.x, velocity.y, velocity.z));
}

Grindstone::Math::Float3 CharacterRigidbodyControllerComponent::GetVelocity() const {
	JPH::Vec3 v = character->GetLinearVelocity();
	return Math::Float3(v.GetX(), v.GetY(), v.GetZ());
}

void CharacterRigidbodyControllerComponent::SetRotation(Math::Quaternion rot) {
	character->SetRotation(JPH::Quat(rot.w, rot.x, rot.y, rot.z));
}

Grindstone::Math::Quaternion CharacterRigidbodyControllerComponent::GetRotation() const {
	JPH::Quat v = character->GetRotation();
	return Math::Quaternion(v.GetY(), v.GetZ(), v.GetW(), v.GetX());
}

CharacterRigidbodyControllerComponent::~CharacterRigidbodyControllerComponent() {
	if (character != nullptr) {
		character->RemoveFromPhysicsSystem();
		character = nullptr;
	}
}

extern "C" {
	JOLT_PHYSICS_EXPORT void* EntityGetCharacterRigidbodyControllerComponent(Grindstone::SceneManagement::Scene* scene, uint32_t entity) {
		entt::registry& reg = Grindstone::EngineCore::GetInstance().GetEntityRegistry();
		const entt::entity entityId = static_cast<entt::entity>(entity);
		CharacterRigidbodyControllerComponent* comp = reg.try_get<CharacterRigidbodyControllerComponent>(entityId);
		return comp;
	}
	
	JOLT_PHYSICS_EXPORT bool CharacterRigidbodyControllerComponentGetIsOnGround(const CharacterRigidbodyControllerComponent& component) {
		return component.IsOnGround();
	}

	JOLT_PHYSICS_EXPORT Grindstone::Math::ExportableVector CharacterRigidbodyControllerComponentGetVelocity(const CharacterRigidbodyControllerComponent& component) {
		return Grindstone::Math::ExportVector(component.GetVelocity());
	}

	JOLT_PHYSICS_EXPORT void CharacterRigidbodyControllerComponentSetVelocity(CharacterRigidbodyControllerComponent& component, Grindstone::Math::ExportableVector velocity) {
		component.SetVelocity(Grindstone::Math::ImportVector(velocity));
	}

	JOLT_PHYSICS_EXPORT Grindstone::Math::ExportableQuaternion CharacterRigidbodyControllerComponentGetRotation(const CharacterRigidbodyControllerComponent& component) {
		return Grindstone::Math::ExportQuaternion(component.GetRotation());
	}

	JOLT_PHYSICS_EXPORT void CharacterRigidbodyControllerComponentSetRotation(CharacterRigidbodyControllerComponent& component, Grindstone::Math::ExportableQuaternion rot) {
		component.SetRotation(Grindstone::Math::ImportQuaternion(rot));
	}
}
