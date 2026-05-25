#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Common/Math.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

namespace Grindstone {
	class WorldContextSet;
}

namespace JPH {
	class Character;
}

namespace Grindstone::Physics {
	struct ColliderComponent;

	void SetupCharacterControllerComponent(Grindstone::WorldContextSet&, entt::entity);

	struct CharacterControllerComponent {
		CharacterControllerComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		void SetVelocity(Math::Float3 velocity) const;
	private:
		JPH::Character* character = nullptr;

		REFLECT("CharacterController")
	};

	void SetupCharacterControllerComponentWithCollider(
		Grindstone::WorldContextSet& cxt,
		CharacterControllerComponent* characterControllerComponent,
		TransformComponent* transformComponent,
		ColliderComponent* colliderComponent
	);
}
