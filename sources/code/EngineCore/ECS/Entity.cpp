#include "Entity.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
using namespace Grindstone::ECS;

entt::registry& Entity::GetSceneEntityRegistry() {
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

bool Entity::HasComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->HasComponent(componentType, *this);
}

void* Entity::GetComponent(const char* componentType) {
	void* outComponent = nullptr;
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->TryGetComponent(componentType, *this, outComponent);

	return outComponent;
}

bool Entity::TryGetComponent(const char* componentType, void*& outComponent) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	return componentRegistrar->TryGetComponent(componentType, *this, outComponent);
}

void Entity::RemoveComponent(const char* componentType) {
	ComponentRegistrar* componentRegistrar = scene->GetComponentRegistrar();
	auto& entityRegistry = scene->GetEntityRegistry();
	componentRegistrar->RemoveComponent(componentType, *this);
}

void Entity::Destroy() {
	scene->GetEntityRegistry().destroy(entityId);
	entityId = entt::null;
	scene = nullptr;
}
