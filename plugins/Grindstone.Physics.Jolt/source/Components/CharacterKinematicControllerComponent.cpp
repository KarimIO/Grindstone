#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include <Common/Math.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Physics.Jolt/include/Components/CharacterKinematicControllerComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

using namespace JPH::literals;

using namespace Grindstone::Physics;
using namespace Grindstone::Math;

REFLECT_STRUCT_BEGIN(CharacterKinematicControllerComponent)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()

// TODO: CharacterVirtual is really not handled thoroughly, I just wanted a stub to remember where to pick this up later on.
void Grindstone::Physics::SetupCharacterKinematicControllerComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity) {
	entt::registry& registry = cxtSet.GetEntityRegistry();
	ColliderComponent* colliderComponent = GetCollider(registry, entity);
	if (colliderComponent == nullptr) {
		return;
	}

	CharacterKinematicControllerComponent& CharacterKinematicControllerComponent = registry.get<Physics::CharacterKinematicControllerComponent>(entity);
	TransformComponent& transformComponent = registry.get<TransformComponent>(entity);

	SetupCharacterKinematicControllerComponentWithCollider(cxtSet, &CharacterKinematicControllerComponent, &transformComponent, colliderComponent);
}

void Grindstone::Physics::SetupCharacterKinematicControllerComponentWithCollider(
	Grindstone::WorldContextSet& cxtSet,
	CharacterKinematicControllerComponent* characterComponent,
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

	JPH::CharacterVirtualSettings settings{};
	settings.mShape = shape;
	settings.mMass = 80.0f;

	Grindstone::Physics::WorldContext* physWorldContext = static_cast<Grindstone::Physics::WorldContext*>(cxtSet.GetContext(physicsWorldContextName));
	if (physWorldContext != nullptr) {
		JPH::uint64 userParameter = 0;
		JPH::PhysicsSystem* physSystem = &physWorldContext->GetPhysicsSystem();
		characterComponent->character = new JPH::CharacterVirtual(&settings, position, rotation, userParameter, physSystem);
		// TODO: Add character to world
	}
}

CharacterKinematicControllerComponent CharacterKinematicControllerComponent::Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const {
	CharacterKinematicControllerComponent character{};

	return character;
}

void CharacterKinematicControllerComponent::SetVelocity(Math::Float3 velocity) const {
	character->SetLinearVelocity(JPH::Vec3(velocity.x, velocity.y, velocity.z));
}
