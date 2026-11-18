#pragma once

#include <Common/HashedString.hpp>
#include <Common/Math.hpp>

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include "EntityHandle.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace ECS {
		class Entity {
		private:
			EntityHandle entityId = entt::null;
			Grindstone::WorldContextSet* worldContextSet = nullptr;
		public:
			Entity() = default;
			Entity(const Entity& other) = default;
			Entity(entt::entity entityId, Grindstone::WorldContextSet* worldContextSet)
				: entityId(entityId), worldContextSet(worldContextSet) {}

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
			virtual entt::registry& GetSceneEntityRegistry() const;

			template<typename ComponentType, typename... Args>
			ComponentType& AddComponent(Args&&... args) {
				return GetSceneEntityRegistry().emplace<ComponentType>(entityId, std::forward<Args>(args)...);
			}

			template<typename ComponentType>
			bool HasComponent() const {
				return GetSceneEntityRegistry().all_of<ComponentType>(entityId);
			}

			template<typename ComponentType>
			ComponentType& GetComponent() const {
				return GetSceneEntityRegistry().get<ComponentType>(entityId);
			}

			template<typename ComponentType>
			bool TryGetComponent(ComponentType*& outComponent) const {
				ComponentType* testComponent = GetSceneEntityRegistry().try_get<ComponentType>(entityId);
				if (testComponent != nullptr) {
					outComponent = testComponent;
					return true;
				}

				return false;
			}

			template<typename ComponentType>
			void RemoveComponent() {
				GetSceneEntityRegistry().remove<ComponentType>(entityId);
			}

			virtual EntityHandle GetHandle() const {
				return entityId;
			}

			virtual Grindstone::WorldContextSet* GetWorldContextSet() const {
				return worldContextSet;
			}

			explicit operator bool() const {
				return entityId != entt::null &&
						worldContextSet != nullptr &&
						GetSceneEntityRegistry().valid(entityId);
			}

			bool operator==(const Entity& other) const {
				return (entityId == other.entityId) && (worldContextSet == other.worldContextSet);
			}

			bool operator!=(const Entity& other) const {
				return !(*this == other);
			}
		};

		inline bool operator < (const ECS::Entity& lhs, const ECS::Entity& rhs) {
			const bool isCxtSetLess = lhs.GetWorldContextSet() < rhs.GetWorldContextSet();
			const bool isEntityLess = lhs.GetHandle() < rhs.GetHandle();
			return isCxtSetLess || isEntityLess;
		}
	}
}
