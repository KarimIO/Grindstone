#include <stdexcept>
#include <iostream>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

Scene::Scene(
	ECS::ComponentRegistrar* componentRegistrar,
	ECS::SystemRegistrar* systemRegistrar
) : componentRegistrar(componentRegistrar), systemRegistrar(systemRegistrar) {}

ECS::Entity Scene::createEntity() {
	return registry.create();
}

entt::registry* Scene::getEntityRegistry() {
	return &registry;
}

ECS::ComponentRegistrar* Grindstone::SceneManagement::Scene::getComponentRegistrar() {
	return componentRegistrar;
}

void Scene::update() {
	systemRegistrar->update(registry);
}
