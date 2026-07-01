#pragma once

#include <Common/Math.hpp>
#include <Common/HashedString.hpp>

#include "EntityHandle.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace ECS {
		class Entity {
		private:
			EntityHandle entityId = entt::null;
			WorldContextSet* cxtSet = nullptr;
			virtual entt::registry& GetWorldContextSetEntityRegistry() const;
		public:
			Entity() = default;
			Entity(const Entity& other) = default;
			Entity(entt::entity entityId, WorldContextSet * cxtSet)
				: entityId(entityId), cxtSet(cxtSet) {}

			virtual void* AddComponent(Grindstone::HashedString componentType);
			virtual void* AddComponentWithoutSetup(Grindstone::HashedString componentType);
			virtual bool HasComponent(Grindstone::HashedString componentType) const;
			virtual void* GetComponent(Grindstone::HashedString componentType) const;
			virtual bool TryGetComponent(Grindstone::HashedString componentType, void*& outComponent) const;
			virtual void RemoveComponent(Grindstone::HashedString componentType);

			// Parent helpers
			virtual bool IsChildOf(const Entity& other) const;
			virtual Entity GetParent() const;
			virtual bool SetParent(Entity);

			// Transform helpers
			virtual Math::Matrix4 GetLocalMatrix() const;
			virtual Math::Matrix4 GetWorldMatrix() const;
			virtual Math::Float3 GetLocalPosition() const;
			virtual Math::Float3 GetWorldPosition() const;
			virtual Math::Quaternion GetLocalRotation() const;
			virtual Math::Quaternion GetWorldRotation() const;
			virtual Math::Float3 GetLocalScale() const;

			virtual Math::Float3 GetLocalForward() const;
			virtual Math::Float3 GetWorldForward() const;
			virtual Math::Float3 GetLocalRight() const;
			virtual Math::Float3 GetWorldRight() const;
			virtual Math::Float3 GetLocalUp() const;
			virtual Math::Float3 GetWorldUp() const;

			virtual void Destroy();

			template<typename ComponentType, typename... Args>
			ComponentType& AddComponent(Args&&... args) {
				return GetWorldContextSetEntityRegistry().emplace<ComponentType>(entityId, std::forward<Args>(args)...);
			}

			template<typename ComponentType>
			bool HasComponent() const {
				return GetWorldContextSetEntityRegistry().all_of<ComponentType>(entityId);
			}

			template<typename ComponentType>
			ComponentType& GetComponent() const {
				return GetWorldContextSetEntityRegistry().get<ComponentType>(entityId);
			}

			template<typename ComponentType>
			bool TryGetComponent(ComponentType*& outComponent) const {
				ComponentType* testComponent = GetWorldContextSetEntityRegistry().try_get<ComponentType>(entityId);
				if (testComponent != nullptr) {
					outComponent = testComponent;
					return true;
				}

				return false;
			}

			template<typename ComponentType>
			void RemoveComponent() {
				GetWorldContextSetEntityRegistry().remove<ComponentType>(entityId);
			}

			virtual EntityHandle GetHandle() const {
				return entityId;
			}

			virtual WorldContextSet* GetWorldContextSet() const {
				return cxtSet;
			}

			explicit operator bool() const {
				return entityId != entt::null && cxtSet != nullptr && GetWorldContextSetEntityRegistry().valid(entityId);
			}

			bool operator==(const Entity& other) const {
				return (entityId == other.entityId) && (cxtSet == other.cxtSet);
			}

			bool operator!=(const Entity& other) const {
				return !(*this == other);
			}
		};

		inline bool operator < (const ECS::Entity& lhs, const ECS::Entity& rhs) {
			const bool isWorldContextSetLess = lhs.GetWorldContextSet() < rhs.GetWorldContextSet();
			const bool isEntityLess = lhs.GetHandle() < rhs.GetHandle();
			return isWorldContextSetLess || isEntityLess;
		};
	}
}
