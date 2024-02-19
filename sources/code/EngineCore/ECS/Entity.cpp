#include "Entity.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone::ECS;

entt::registry& Entity::GetSceneEntityRegistry() const {
	return scene->GetEntityRegistry();
}

void* Entity::AddComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	return componentRegistrar->CreateComponentWithSetup(componentType, *this);
}

void* Entity::AddComponentWithoutSetup(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	return componentRegistrar->CreateComponent(componentType, *this);
}

bool Entity::HasComponent(const char* componentType) const {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	return componentRegistrar->HasComponent(componentType, *this);
}

void* Entity::GetComponent(const char* componentType) const {
	void* outComponent = nullptr;
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	componentRegistrar->TryGetComponent(componentType, *this, outComponent);

	return outComponent;
}

bool Entity::TryGetComponent(const char* componentType, void*& outComponent) const {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	return componentRegistrar->TryGetComponent(componentType, *this, outComponent);
}

void Entity::RemoveComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
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
		const ParentComponent* parentComponent = registry.try_get<ParentComponent>(currentNode);
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
	const entt::entity parentNode = registry.get<ParentComponent>(entityId).parentEntity;
	return Entity(parentNode, scene);
}

bool Entity::SetParent(const Entity newParent) {
	const bool thisEntityIsChildOfNewParent = newParent.IsChildOf(*this);
	if (thisEntityIsChildOfNewParent || entityId == newParent.entityId) {
		return false;
	}

	TransformComponent& transformComponent = GetComponent<TransformComponent>();
	auto& [parentEntity] = GetComponent<ParentComponent>();
	const Math::Matrix4 currentWorldMatrix = GetWorldMatrix();

	if (!newParent) {
		transformComponent.SetLocalMatrix(currentWorldMatrix);
		parentEntity = entt::null;
		return true;
	}

	transformComponent.SetWorldMatrixRelativeTo(currentWorldMatrix, newParent.GetWorldMatrix());
	parentEntity = newParent.entityId;

	return true;
}

Math::Matrix4 Entity::GetLocalMatrix() const {
	entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetTransformMatrix();
}

Math::Matrix4 Entity::GetWorldMatrix() const {
	return TransformComponent::GetWorldTransformMatrix(*this);
}

Math::Float3 Entity::GetLocalPosition() const {
	entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.position;
}

Math::Float3 Entity::GetWorldPosition() const {
	return TransformComponent::GetWorldPosition(*this);
}

Math::Quaternion Entity::GetLocalRotation() const {
	entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.rotation;
}

Math::Quaternion Entity::GetWorldRotation() const {
	const Math::Matrix4 matrix = GetWorldMatrix();
	return { matrix };
}

Math::Float3 Entity::GetLocalScale() const {
	const entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.scale;
}


Math::Float3 Entity::GetLocalForward() const {
	entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetForward();
}

Math::Float3 Entity::GetWorldForward() const {
	const Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(0.0f, 0.0f, 1.0f);
}

Math::Float3 Entity::GetLocalRight() const {
	entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetRight();
}

Math::Float3 Entity::GetWorldRight() const {
	const Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(1.0f, 0.0f, 0.0f);
}

Math::Float3 Entity::GetLocalUp() const {
	entt::registry& registry = GetSceneEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetUp();
}

Math::Float3 Entity::GetWorldUp() const {
	const Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(0.0f, 1.0f, 0.0f);
}


void Entity::Destroy() {
	scene->GetEntityRegistry().destroy(entityId);
	entityId = entt::null;
	scene = nullptr;
}
