#pragma once

#include <entt/entt.hpp>
#include "EntityHandle.hpp"

#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>

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
			virtual bool IsChildOf(const Entity& other) const;
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
				return GetSceneEntityRegistry().remove<ComponentType>(entityId);
			}

			virtual EntityHandle GetHandle() const {
				return entityId;
			}

			virtual SceneManagement::Scene* GetScene() const {
				return scene;
			}

			operator bool() const {
				return entityId == entt::null;
			}

			bool Entity::operator==(const Entity other) const {
				return (entityId == other.entityId) && (scene == other.scene);
			}

			bool Entity::operator!=(const Entity other) const {
				return !(*this == other);
			}
		};

		inline bool operator < (const ECS::Entity& lhs, const ECS::Entity& rhs) {
			bool isSceneLess = lhs.GetScene() < rhs.GetScene();
			bool isEntityLess = lhs.GetHandle() < rhs.GetHandle();
			return isSceneLess || isEntityLess;
		}
	}
}
