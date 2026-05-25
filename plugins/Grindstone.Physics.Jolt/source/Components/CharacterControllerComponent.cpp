#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Character/Character.h>

#include <Common/Math.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Physics.Jolt/include/Components/CharacterControllerComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

using namespace JPH::literals;

using namespace Grindstone::Physics;
using namespace Grindstone::Math;

REFLECT_STRUCT_BEGIN(CharacterControllerComponent)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

void Grindstone::Physics::SetupCharacterControllerComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	entt::registry& registry = cxtSet.GetEntityRegistry();
	ColliderComponent* colliderComponent = GetCollider(registry, entity);
	if (colliderComponent == nullptr) {
		return;
	}

	CharacterControllerComponent& CharacterControllerComponent = registry.get<Physics::CharacterControllerComponent>(entity);
	TransformComponent& transformComponent = registry.get<TransformComponent>(entity);

	SetupCharacterControllerComponentWithCollider(cxtSet, &CharacterControllerComponent, &transformComponent, colliderComponent);
}

void Grindstone::Physics::SetupCharacterControllerComponentWithCollider(
	Grindstone::WorldContextSet& cxtSet,
	CharacterControllerComponent* CharacterControllerComponent,
	TransformComponent* transformComponent,
	ColliderComponent* colliderComponent
) {
	JPH::Quat rotation = JPH::Quat(
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

	JPH::CharacterSettings settings;
	settings.mShape = shape;
	settings.mLayer = Layers::MOVING;
	settings.mMass = 80.0f;

	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet.GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		JPH::uint64 userParameter = 0;
		JPH::PhysicsSystem* physSystem = &physWorldContext->GetPhysicsSystem();
		JPH::Character* character = new JPH::Character(&settings, position, rotation, userParameter, physSystem);
		character->AddToPhysicsSystem();
	}
}

CharacterControllerComponent CharacterControllerComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	CharacterControllerComponent character{};

	return character;
}

void CharacterControllerComponent::SetVelocity(Math::Float3 velocity) const {
	character->SetLinearVelocity(JPH::Vec3(velocity.x, velocity.y, velocity.z));
}
