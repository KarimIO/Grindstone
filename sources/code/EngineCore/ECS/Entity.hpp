#pragma once

#include <entt/entt.hpp>

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace ECS {
		using EntityHandle = entt::entity;
		const auto EmptyEntityHandle = entt::null;

		class Entity {
		public:
			Entity() = default;
			Entity(const Entity& other) = default;
			Entity::Entity(entt::entity entityId, SceneManagement::Scene * scene)
				: entityId(entityId), scene(scene) {}

			virtual void* AddComponent(const char* componentType);
			virtual bool HasComponent(const char* componentType);
			virtual void* GetComponent(const char* componentType);
			virtual bool TryGetComponent(const char* componentType, void*& outComponent);
			virtual void RemoveComponent(const char* componentType);
			virtual void Destroy();

			template<typename ComponentType, typename... Args>
			ComponentType& AddComponent(Args&&... args) {
				return scene->GetEntityRegistry().emplace<ComponentType>(entityId, std::forward<Args>(args)...);
			}
			template<typename ComponentType>
			bool HasComponent() {
				return scene->GetEntityRegistry().has<ComponentType>(entityId);
			}
			template<typename ComponentType>
			ComponentType& GetComponent() {
				return scene->GetEntityRegistry().get<ComponentType>(entityId);
			}
			template<typename ComponentType>
			bool TryGetComponent(ComponentType*& outComponent) {
				auto& registry = scene->GetEntityRegistry();
				if (registry.has<ComponentType>(entityId)) {
					outComponent = &registry.get<ComponentType>(entityId);
					return true;
				}

				outComponent = nullptr;
				return false;
			}
			template<typename ComponentType>
			void RemoveComponent(const char* componentType) {
				return scene->GetEntityRegistry().remove<ComponentType>(entityId);
			}

			virtual EntityHandle GetHandle() const {
				return entityId;
			}

			virtual SceneManagement::Scene* GetScene() const {
				return scene;
			}

			operator bool() {
				return entityId == entt::null;
			}

			bool Entity::operator==(const Entity other) {
				return (entityId == other.entityId) && (scene = other.scene);
			}

			bool Entity::operator!=(const Entity other) {
				return !(*this == other);
			}
		private:
			EntityHandle entityId = entt::null;
			SceneManagement::Scene* scene = nullptr;
		};

		inline bool operator< (const ECS::Entity& lhs, const ECS::Entity& rhs) {
			bool isSceneLess = lhs.GetScene() < rhs.GetScene();
			bool isEntityLess = lhs.GetHandle() < rhs.GetHandle();
			return isSceneLess || isEntityLess;
		}
	}
}
