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
	class CharacterVirtual;
}

namespace Grindstone::Physics {
	struct ColliderComponent;

	void SetupCharacterKinematicControllerComponent(Grindstone::WorldContextSet&, entt::entity);

	struct CharacterKinematicControllerComponent {
		CharacterKinematicControllerComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		void SetVelocity(Math::Float3 velocity) const;

		JPH::Ref<JPH::CharacterVirtual> character = nullptr;

		REFLECT("CharacterKinematicController")
	};

	void SetupCharacterKinematicControllerComponentWithCollider(
		Grindstone::WorldContextSet& cxt,
		CharacterKinematicControllerComponent* characterControllerComponent,
		TransformComponent* transformComponent,
		ColliderComponent* colliderComponent
	);
}
