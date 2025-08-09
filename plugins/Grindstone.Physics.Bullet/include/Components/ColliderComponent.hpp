#pragma once

#include <bullet/btBulletCollisionCommon.h>
#include <Common/Memory/SmartPointers/UniquePtr.hpp>
#include <Common/Math.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>

namespace Grindstone {
	class WorldContextSet;
}

namespace Grindstone::Physics {
	struct ColliderComponent {
		virtual void Initialize() = 0;

		Grindstone::UniquePtr<btCollisionShape> collisionShape = nullptr;
	};

	struct SphereColliderComponent : public ColliderComponent {
		SphereColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize() override;
		virtual void SetRadius(float radius);
		virtual float GetRadius() const;
	private:
		float radius = 0.0f;

		REFLECT("SphereCollider")
	};

	struct PlaneColliderComponent : public ColliderComponent {
		PlaneColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize() override;
		virtual void SetCollider(Math::Float3 planeNormal, float positionAlongNormal);
		virtual Math::Float3 GetPlaneNormal() const;
		virtual float GetPositionAlongNormal() const;
	private:
		Math::Float3 planeNormal;
		float positionAlongNormal;

		REFLECT("PlaneCollider")
	};

	struct BoxColliderComponent : public ColliderComponent {
		BoxColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize() override;
		virtual void SetSize(Math::Float3);
		virtual Math::Float3 GetSize() const;
	private:
		Math::Float3 size;

		REFLECT("BoxCollider")
	};

	struct CapsuleColliderComponent : public ColliderComponent {
		CapsuleColliderComponent Clone(Grindstone::WorldContextSet& cxt, entt::entity newEntityId) const;
		virtual void Initialize() override;
		virtual void SetCollider(float radius, float height);
		virtual float GetRadius() const;
		virtual float GetHeight() const;
	private:
		float radius;
		float height;

		REFLECT("CapsuleCollider")
	};
}
