#include <stdexcept>
#include <iostream>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "SceneLoaderJson.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

SceneLoaderJson::SceneLoaderJson(Scene* scene, const char* path) : scene(scene), path(path) {
	load(path);
}

ECS::Entity SceneLoaderJson::createEntity() {
	return scene->createEntity();
}

bool SceneLoaderJson::attachComponent(ECS::Entity entity, const char* componentName) {
	auto registry = scene->getEntityRegistry();
	auto componentRegistrar = scene->getComponentRegistrar();
	auto componentFactory = componentRegistrar->createComponent(componentName, *registry, entity);

	return componentFactory != nullptr;
}

bool SceneLoaderJson::load(const char* path) {
	this->path = path;

	// Get from text

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
	ECS::Entity entity = createEntity();
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
