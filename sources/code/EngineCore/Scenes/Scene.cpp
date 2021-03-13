#include <stdexcept>
#include <iostream>

#include "Scene.hpp"

using namespace Grindstone;

Scene::Scene() {
}

ECS::Entity Scene::createEntity() {
	return registry.create();
}

bool Scene::attachComponent(ECS::Entity entity, const char* componentName) {
	auto componentFactory = componentFactories[componentName];
	componentFactory(entity);
}

bool Scene::load(const char* path) {
	return loadFromText(path);
}

bool Scene::loadFromText(const char* path) {
	this->path = path;

	// Get from text
	this->name = path;

	// Register Systems
	//ecs_.registerSystem("PhysicsSystem");
	//ecs_.registerSystem("TransformSystem");

	// Register Components
	/*ecs_.registerComponentType("Collider");
	ecs_.registerComponentType("RigidBody");
	ecs_.registerComponentType("Position");
	ecs_.registerComponentType("Rotation");
	ecs_.registerComponentType("Scale");
	ecs_.registerComponentType("WorldMatrix");
	ecs_.registerComponentType("Heirarchy");*/

	// Load Entities with components
	// - For each Entity
	Entity entity = createEntity();
	// --- For each component
	attachComponent(entity, "Transform");

	/*
	ecs_.createComponent("Rotation");
	ecs_.createComponent("Scale");
	ecs_.createComponent("World");
	ecs_.createComponent("Hierarchy");
	ecs_.createComponent("Hierarchy");
	*/
	// --- 

	return true;
}

bool Scene::loadFromBinary(const char* path) {
	throw std::runtime_error("Scene::loadFromBinary is not implemented!");

	return true;
}

entt::registry* Scene::getEntityRegistry() {
	return &registry;
}

void Scene::update() {
}
