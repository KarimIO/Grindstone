#pragma once

#include <bullet/btBulletCollisionCommon.h>
#include "Common/Math.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	namespace Physics {
		void SetupColliderComponent(ECS::Entity& entity, void* componentPtr);

		struct ColliderComponent {
			virtual void Initialize() = 0;

			btCollisionShape* collisionShape = nullptr;
		};


		struct SphereColliderComponent : public ColliderComponent {
			SphereColliderComponent() = default;
			SphereColliderComponent(float radius);
			virtual void Initialize() override;
			void SetRadius(float radius);
			float GetRadius();
		private:
			float radius;

			REFLECT("SphereCollider")
		};

		struct PlaneColliderComponent : public ColliderComponent {
			PlaneColliderComponent() = default;
			PlaneColliderComponent(Math::Float3 planeNormal, float positionAlongNormal);
			virtual void Initialize() override;
			void SetCollider(Math::Float3 planeNormal, float positionAlongNormal);
			Math::Float3 GetPlaneNormal();
			float GetPositionAlongNormal();
		private:
			Math::Float3 planeNormal;
			float positionAlongNormal;

			REFLECT("PlaneCollider")
		};

		struct BoxColliderComponent : public ColliderComponent {
			BoxColliderComponent() = default;
			BoxColliderComponent(Math::Float3 size);
			virtual void Initialize() override;
			void SetSize(Math::Float3);
			Math::Float3 GetSize();
		private:
			Math::Float3 size;

			REFLECT("BoxCollider")
		};

		struct CapsuleColliderComponent : public ColliderComponent {
			CapsuleColliderComponent() = default;
			CapsuleColliderComponent(float radius, float height);
			virtual void Initialize() override;
			void SetCollider(float radius, float height);
			float GetRadius();
			float GetHeight();
		private:
			float radius;
			float height;

			REFLECT("CapsuleCollider")
		};
	}
}
