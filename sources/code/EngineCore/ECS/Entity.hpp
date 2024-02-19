#pragma once

#include <Common/Math.hpp>
#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>

#include "EntityHandle.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace ECS {
		class Entity {
		private:
			EntityHandle entityId = entt::null;
			SceneManagement::Scene* scene = nullptr;
		public:
			Entity() = default;
			Entity(const Entity& other) = default;
			Entity(entt::entity entityId, SceneManagement::Scene * scene)
				: entityId(entityId), scene(scene) {}

			virtual void* AddComponent(const char* componentType);
			virtual void* AddComponentWithoutSetup(const char* componentType);
			virtual bool HasComponent(const char* componentType) const;
			virtual void* GetComponent(const char* componentType) const;
			virtual bool TryGetComponent(const char* componentType, void*& outComponent) const;
			virtual void RemoveComponent(const char* componentType);

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
			void RemoveComponent(const char* componentType) {
				GetSceneEntityRegistry().remove<ComponentType>(entityId);
			}

			virtual EntityHandle GetHandle() const {
				return entityId;
			}

			virtual SceneManagement::Scene* GetScene() const {
				return scene;
			}

			explicit operator bool() const {
				return entityId != entt::null && scene != nullptr && GetSceneEntityRegistry().valid(entityId);
			}

			bool operator==(const Entity& other) const {
				return (entityId == other.entityId) && (scene == other.scene);
			}

			bool operator!=(const Entity& other) const {
				return !(*this == other);
			}
		};

		inline bool operator < (const ECS::Entity& lhs, const ECS::Entity& rhs) {
			const bool isSceneLess = lhs.GetScene() < rhs.GetScene();
			const bool isEntityLess = lhs.GetHandle() < rhs.GetHandle();
			return isSceneLess || isEntityLess;
		}
	}
}
