#pragma once

#include "Common/Math.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "ColliderComponent.hpp"

class btRigidBody;

namespace Grindstone {
	struct TransformComponent;
	class WorldContextSet;

	namespace Physics {
		struct ColliderComponent;

		void SetupRigidBodyComponent(Grindstone::WorldContextSet&, entt::entity);
		void DestroyRigidBodyComponent(Grindstone::WorldContextSet&, entt::entity);

		struct RigidBodyComponent {
			RigidBodyComponent() = default;
			RigidBodyComponent(float mass, ColliderComponent* colliderComponent);
			RigidBodyComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;

			void SetCollisionShape(ColliderComponent* colliderComponent);
			void SetFriction(float friction);
			void SetRestitution(float restitution);
			void SetDamping(float linear, float rotational);

			void ApplyForce(Math::Float3 pos, Math::Float3 force);
			void ApplyCentralForce(Math::Float3 force);
			void ApplyImpulse(Math::Float3 pos, Math::Float3 force);
			void ApplyCentralImpulse(Math::Float3 force);

			float GetMass() const;
			float GetFriction() const;
			float GetRestitution() const;
			float GetDampingLinear() const;
			float GetDampingRotational() const;
		private:
			float mass = 0.0f;
			float friction = 0.0f;
			float restitution = 0.0f;
			float dampingLinear = 0.0f;
			float dampingRotational = 0.0f;
		public:
			Grindstone::UniquePtr<btRigidBody> rigidBody = nullptr;

			REFLECT("RigidBody")
		};

		void SetupRigidBodyComponentWithCollider(
			Grindstone::WorldContextSet& cxt,
			RigidBodyComponent* rigidBodyComponent,
			TransformComponent* transformComponent,
			ColliderComponent* colliderComponent
		);
	}
}
