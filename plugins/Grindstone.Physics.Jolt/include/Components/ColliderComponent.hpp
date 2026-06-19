#pragma once

#include <entt/entt.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Common/Math.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

namespace Grindstone {
	class WorldContextSet;
}

namespace Grindstone::Physics {
	struct ColliderComponent {
		virtual void Initialize(const TransformComponent& transformComponent) = 0;

		JPH::Ref<JPH::Shape> collisionShape = nullptr;
	};

	ColliderComponent* GetCollider(entt::registry& registry, entt::entity entityHandle);

	struct SphereColliderComponent : public ColliderComponent {
		static void Construct(Grindstone::WorldContextSet& cxt, entt::entity newEntityId);
		SphereColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize(const TransformComponent& transformComponent) override;
		virtual void SetRadius(float radius);
		virtual float GetRadius() const;
	private:
		float radius = 1.0f;

		REFLECT("SphereCollider")
	};

	struct PlaneColliderComponent : public ColliderComponent {
		static void Construct(Grindstone::WorldContextSet& cxt, entt::entity newEntityId);
		PlaneColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize(const TransformComponent& transformComponent) override;
		virtual void SetCollider(Math::Float3 planeNormal, float positionAlongNormal);
		virtual Math::Float3 GetPlaneNormal() const;
		virtual float GetPositionAlongNormal() const;
	private:
		Math::Float3 planeNormal = Math::Float3(0.0f, 0.0f, 1.0f);
		float positionAlongNormal = 0.0f;

		REFLECT("PlaneCollider")
	};

	struct BoxColliderComponent : public ColliderComponent {
		static void Construct(Grindstone::WorldContextSet& cxt, entt::entity newEntityId);
		BoxColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize(const TransformComponent& transformComponent) override;
		virtual void SetSize(Math::Float3);
		virtual Math::Float3 GetSize() const;
	private:
		Math::Float3 size = Math::Float3(1.0f, 1.0f, 1.0f);

		REFLECT("BoxCollider")
	};

	struct CapsuleColliderComponent : public ColliderComponent {
		static void Construct(Grindstone::WorldContextSet& cxt, entt::entity newEntityId);
		CapsuleColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize(const TransformComponent& transformComponent) override;
		virtual void SetCollider(float radius, float height);
		virtual float GetRadius() const;
		virtual float GetHeight() const;
	private:
		float radius = 0.5f;
		float height = 2.0f;

		REFLECT("CapsuleCollider")
	};
}
