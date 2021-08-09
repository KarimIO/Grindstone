#include <stdexcept>
#include <iostream>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

Scene::Scene(
	EngineCore* engineCore,
	ECS::ComponentRegistrar* componentRegistrar,
	ECS::SystemRegistrar* systemRegistrar
) : engineCore(engineCore), componentRegistrar(componentRegistrar), systemRegistrar(systemRegistrar) {}

ECS::Entity Scene::CreateEmptyEntity() {
	entt::entity entityId = registry.create();
	return { entityId, this };
}

void Scene::DestroyEntity(ECS::EntityHandle entityId) {
	registry.destroy(entityId);
}

void Scene::DestroyEntity(ECS::Entity entity) {
	DestroyEntity(entity.GetHandle());
}

ECS::Entity Grindstone::SceneManagement::Scene::CreateEntity() {
	auto entity = CreateEmptyEntity();
	entity.AddComponent<TagComponent>("New Entity");
	entity.AddComponent<TransformComponent>();
	return entity;
}

const char* Grindstone::SceneManagement::Scene::GetName() {
	return name.c_str();
}

entt::registry& Scene::GetEntityRegistry() {
	return registry;
}

ECS::ComponentRegistrar* Grindstone::SceneManagement::Scene::GetComponentRegistrar() {
	return componentRegistrar;
}

void Scene::Update() {
	systemRegistrar->Update(engineCore, registry);
}
