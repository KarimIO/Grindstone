#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Character/Character.h>

#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Common/Math.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

namespace Grindstone {
	class WorldContextSet;
}

namespace Grindstone::Physics {
	struct ColliderComponent;

	void SetupCharacterRigidbodyControllerComponent(Grindstone::WorldContextSet&, entt::entity);

	struct CharacterRigidbodyControllerComponent {
		CharacterRigidbodyControllerComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		bool IsOnGround() const;
		~CharacterRigidbodyControllerComponent();
		void SetVelocity(Math::Float3 velocity);
		Grindstone::Math::Float3 GetVelocity() const;
		void SetRotation(Math::Quaternion rot);
		Grindstone::Math::Quaternion GetRotation() const;

		JPH::Ref<JPH::Character> character = nullptr;

		REFLECT("CharacterRigidbodyController")
	};

	void SetupCharacterRigidbodyControllerComponentWithCollider(
		Grindstone::WorldContextSet& cxt,
		CharacterRigidbodyControllerComponent* characterControllerComponent,
		TransformComponent* transformComponent,
		ColliderComponent* colliderComponent
	);
}
