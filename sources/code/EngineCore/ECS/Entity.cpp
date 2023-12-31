#include "Entity.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
using namespace Grindstone::ECS;

entt::registry& Entity::GetSceneEntityRegistry() const {
	return scene->GetEntityRegistry();
}

void* Entity::AddComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	entt::registry& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->CreateComponentWithSetup(componentType, *this);
}

void* Entity::AddComponentWithoutSetup(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->CreateComponent(componentType, *this);
}

bool Entity::HasComponent(const char* componentType) const {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	entt::registry& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->HasComponent(componentType, *this);
}

void* Entity::GetComponent(const char* componentType) const {
	void* outComponent = nullptr;
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	entt::registry& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->TryGetComponent(componentType, *this, outComponent);

	return outComponent;
}

bool Entity::TryGetComponent(const char* componentType, void*& outComponent) const {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	entt::registry& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->TryGetComponent(componentType, *this, outComponent);
}

void Entity::RemoveComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	entt::registry& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->RemoveComponent(componentType, *this);
}

bool Entity::IsChildOf(const Entity& possibleParent) const {
	if (scene != possibleParent.scene) {
		return false;
	}

	if (entityId == possibleParent.entityId) {
		return false;
	}

	entt::registry& registry = GetSceneEntityRegistry();
	entt::entity currentNode = possibleParent.entityId;
	while (currentNode != entt::null) {
		ParentComponent* parentComponent = registry.try_get<ParentComponent>(currentNode);
		if (parentComponent == nullptr) {
			return false;
		}

		if (parentComponent->parentEntity == entityId) {
			return true;
		}

		currentNode = parentComponent->parentEntity;
	}

	return false;
}

Entity Entity::GetParent() const {
	entt::registry& registry = GetSceneEntityRegistry();
	entt::entity parentNode = registry.get<ParentComponent>(entityId).parentEntity;
	return Entity(parentNode, scene);
}

Math::Matrix4 Entity::GetLocalMatrix() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetTransformMatrix();
}

Math::Matrix4 Entity::GetWorldMatrix() const {
	return TransformComponent::GetWorldTransformMatrix(*this);
}

Math::Float3 Entity::GetLocalPosition() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.position;
}

Math::Float3 Entity::GetWorldPosition() const {
	return TransformComponent::GetWorldPosition(*this);
}

Math::Quaternion Entity::GetLocalRotation() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.rotation;
}

Math::Quaternion Entity::GetWorldRotation() const {
	Math::Matrix4 matrix = GetWorldMatrix();
	return Math::Quaternion(matrix);
}

Math::Float3 Entity::GetLocalScale() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.scale;
}


Math::Float3 Entity::GetLocalForward() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetForward();
}

Math::Float3 Entity::GetWorldForward() const {
	Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(0.0f, 0.0f, 1.0f);
}

Math::Float3 Entity::GetLocalRight() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetRight();
}

Math::Float3 Entity::GetWorldRight() const {
	Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(1.0f, 0.0f, 0.0f);
}

Math::Float3 Entity::GetLocalUp() const {
	entt::registry& registry = GetSceneEntityRegistry();
	TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetUp();
}

Math::Float3 Entity::GetWorldUp() const {
	Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(0.0f, 1.0f, 0.0f);
}


void Entity::Destroy() {
	scene->GetEntityRegistry().destroy(entityId);
	entityId = entt::null;
	scene = nullptr;
}
