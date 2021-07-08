#include <stdexcept>
#include <iostream>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

Scene::Scene(
	EngineCore* engineCore,
	ECS::ComponentRegistrar* componentRegistrar,
	ECS::SystemRegistrar* systemRegistrar
) : engineCore(engineCore), componentRegistrar(componentRegistrar), systemRegistrar(systemRegistrar) {}

ECS::Entity Scene::createEntity() {
	return registry.create();
}

void* Scene::attachComponent(ECS::Entity entity, const char* componentName) {
	return componentRegistrar->createComponent(componentName, registry, entity);
}

ECS::Entity Grindstone::SceneManagement::Scene::createDefaultEntity() {
	auto entity = createEntity();
	auto tag = (TagComponent *)attachComponent(entity, "Tag");
	tag->tag = "New Component";
	attachComponent(entity, "Transform");
	return entity;
}

const char* Grindstone::SceneManagement::Scene::getName() {
	return name.c_str();
}

entt::registry* Scene::getEntityRegistry() {
	return &registry;
}

ECS::ComponentRegistrar* Grindstone::SceneManagement::Scene::getComponentRegistrar() {
	return componentRegistrar;
}

void Scene::update() {
	systemRegistrar->Update(engineCore, registry);
}
