#include "Entity.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

entt::registry& Entity::GetSceneEntityRegistry() const {
	return scene->GetEntityRegistry();
}

void* Entity::AddComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->CreateComponentWithSetup(componentType, *this);
}

void* Entity::AddComponentWithoutSetup(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->CreateComponent(componentType, *this);
}

bool Entity::HasComponent(const char* componentType) const {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->HasComponent(componentType, *this);
}

void* Entity::GetComponent(const char* componentType) const {
	void* outComponent = nullptr;
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->TryGetComponent(componentType, *this, outComponent);

	return outComponent;
}

bool Entity::TryGetComponent(const char* componentType, void*& outComponent) const {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->TryGetComponent(componentType, *this, outComponent);
}

void Entity::RemoveComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
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

void Entity::Destroy() {
	scene->GetEntityRegistry().destroy(entityId);
	entityId = entt::null;
	scene = nullptr;
}
