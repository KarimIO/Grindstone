#pragma once

#include "Common/Math.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "ColliderComponent.hpp"

class btRigidBody;

namespace Grindstone {
	struct TransformComponent;

	namespace Physics {
		struct ColliderComponent;

		void SetupRigidBodyComponent(entt::registry&, entt::entity);
		void DestroyRigidBodyComponent(entt::registry&, entt::entity);

		struct RigidBodyComponent {
			RigidBodyComponent() = default;
			RigidBodyComponent(float mass, ColliderComponent* colliderComponent);

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
			btRigidBody* rigidBody = nullptr;

			REFLECT("RigidBody")
		};

		void SetupRigidBodyComponentWithCollider(
			RigidBodyComponent* rigidBodyComponent,
			TransformComponent* transformComponent,
			ColliderComponent* colliderComponent
		);
	}
}
