#include <Common/Assert.hpp>

#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "Entity.hpp"
using namespace Grindstone::ECS;

static ComponentRegistrar* GetComponentRegistrar() {
	ComponentRegistrar* componentRegistrar = Grindstone::EngineCore::GetInstance().GetComponentRegistrar();
	GS_ASSERT(componentRegistrar);
	return componentRegistrar;
}

entt::registry& Entity::GetWorldContextSetEntityRegistry() const {
	return cxtSet->GetEntityRegistry();
}

void* Entity::AddComponent(Grindstone::HashedString componentType) {
	ComponentRegistrar* componentRegistrar = GetComponentRegistrar();
	return componentRegistrar->CreateComponentWithSetup(*cxtSet, componentType, *this);
}

void* Entity::AddComponentWithoutSetup(Grindstone::HashedString componentType) {
	ComponentRegistrar* componentRegistrar = GetComponentRegistrar();
	return componentRegistrar->CreateComponent(*cxtSet, componentType, *this);
}

bool Entity::HasComponent(Grindstone::HashedString componentType) const {
	ComponentRegistrar* componentRegistrar = GetComponentRegistrar();
	return componentRegistrar->HasComponent(*cxtSet, componentType, *this);
}

void* Entity::GetComponent(Grindstone::HashedString componentType) const {
	void* outComponent = nullptr;
	ComponentRegistrar* componentRegistrar = GetComponentRegistrar();
	componentRegistrar->TryGetComponent(*cxtSet, componentType, *this, outComponent);

	return outComponent;
}

bool Entity::TryGetComponent(Grindstone::HashedString componentType, void*& outComponent) const {
	ComponentRegistrar* componentRegistrar = GetComponentRegistrar();
	return componentRegistrar->TryGetComponent(*cxtSet, componentType, *this, outComponent);
}

void Entity::RemoveComponent(Grindstone::HashedString componentType) {
	ComponentRegistrar* componentRegistrar = GetComponentRegistrar();
	componentRegistrar->RemoveComponent(*cxtSet, componentType, *this);
}

bool Entity::IsChildOf(const Entity& possibleParent) const {
	if (cxtSet != possibleParent.cxtSet) {
		return false;
	}

	if (entityId == possibleParent.entityId) {
		return false;
	}

	entt::registry& registry = GetWorldContextSetEntityRegistry();
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
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const entt::entity parentNode = registry.get<ParentComponent>(entityId).parentEntity;
	return Entity(parentNode, cxtSet);
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
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetTransformMatrix();
}

Math::Matrix4 Entity::GetWorldMatrix() const {
	return TransformComponent::GetWorldTransformMatrix(*this);
}

Math::Float3 Entity::GetLocalPosition() const {
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.position;
}

Math::Float3 Entity::GetWorldPosition() const {
	return TransformComponent::GetWorldPosition(*this);
}

Math::Quaternion Entity::GetLocalRotation() const {
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.rotation;
}

Math::Quaternion Entity::GetWorldRotation() const {
	const Math::Matrix4 matrix = GetWorldMatrix();
	return { matrix };
}

Math::Float3 Entity::GetLocalScale() const {
	const entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.scale;
}


Math::Float3 Entity::GetLocalForward() const {
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetForward();
}

Math::Float3 Entity::GetWorldForward() const {
	const Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(0.0f, 0.0f, 1.0f);
}

Math::Float3 Entity::GetLocalRight() const {
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetRight();
}

Math::Float3 Entity::GetWorldRight() const {
	const Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(1.0f, 0.0f, 0.0f);
}

Math::Float3 Entity::GetLocalUp() const {
	entt::registry& registry = GetWorldContextSetEntityRegistry();
	const TransformComponent& transformComponent = registry.get<TransformComponent>(entityId);

	return transformComponent.GetUp();
}

Math::Float3 Entity::GetWorldUp() const {
	const Math::Quaternion rotation = GetWorldRotation();
	return rotation * Math::Float3(0.0f, 1.0f, 0.0f);
}


void Entity::Destroy() {
	cxtSet->GetEntityRegistry().destroy(entityId);
	entityId = entt::null;
	cxtSet = nullptr;
}
