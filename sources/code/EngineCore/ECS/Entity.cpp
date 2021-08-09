#include "Entity.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

void* Entity::AddComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->CreateComponent(componentType, entityRegistry, entityId);
}

bool Entity::HasComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->HasComponent(componentType, entityRegistry, entityId);
}

void* Entity::GetComponent(const char* componentType) {
	void* outComponent = nullptr;
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->TryGetComponent(componentType, entityRegistry, entityId, outComponent);

	return outComponent;
}

bool Entity::TryGetComponent(const char* componentType, void*& outComponent) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->TryGetComponent(componentType, entityRegistry, entityId, outComponent);
}

void Entity::RemoveComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->RemoveComponent(componentType, entityRegistry, entityId);
}

void Entity::Destroy() {
	scene->GetEntityRegistry().destroy(entityId);
	entityId = entt::null;
	scene = nullptr;
}
