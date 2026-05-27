#pragma once

#include <Jolt/Physics/Body/BodyID.h>

#include <Common/Math.hpp>
#include <Common/PhysicsLayer.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>

#include "ColliderComponent.hpp"

class btRigidBody;

namespace Grindstone {
	struct TransformComponent;
	class WorldContextSet;

	namespace Physics {
		struct ColliderComponent;

		void SetupRigidBodyComponent(Grindstone::WorldContextSet&, entt::entity);

		struct RigidBodyComponent {
			RigidBodyComponent() = default;
			RigidBodyComponent(float mass, ColliderComponent* colliderComponent);
			RigidBodyComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
			~RigidBodyComponent();

			void SetIsStatic(bool isStatic);
			void SetLayer(Grindstone::Physics::Layer layer);
			void SetFriction(float friction);
			void SetRestitution(float restitution);
			void SetPosition(Math::Float3 pos);
			void SetRotation(Math::Quaternion rot);

			void ApplyForce(Math::Float3 pos, Math::Float3 force);
			void ApplyCentralForce(Math::Float3 force);
			void ApplyImpulse(Math::Float3 pos, Math::Float3 force);
			void ApplyCentralImpulse(Math::Float3 force);

			bool GetIsStatic() const;
			Grindstone::Physics::Layer GetLayer() const;
			float GetMass() const;
			float GetFriction() const;
			float GetRestitution() const;
			Math::Float3 GetPosition() const;
			Math::Quaternion GetRotation() const;

			void SetBodyID(JPH::BodyID bodyId);
			JPH::BodyID GetBodyID() const;

			bool isStatic = false;
			Grindstone::Physics::Layer layer = 0;

			float mass = 0.0f;
			float friction = 0.0f;
			float restitution = 0.0f;
			JPH::BodyID rigidBody;

		public:

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
