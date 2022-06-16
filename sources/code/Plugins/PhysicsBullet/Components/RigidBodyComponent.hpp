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

		void SetupRigidBodyComponent(ECS::Entity& entity, void* componentPtr);

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

			float GetMass();
			float GetFriction();
			float GetRestitution();
			float GetDampingLinear();
			float GetDampingRotational();
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
