#include <stdexcept>
#include <iostream>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
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

void* SceneLoaderJson::attachComponent(ECS::Entity entity, const char* componentName) {
	return scene->attachComponent(entity, componentName);
}

bool SceneLoaderJson::load(const char* path) {
	scene->path = path;
	scene->name = path;

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
	TagComponent* tag = (TagComponent*)attachComponent(entity, "Tag");
	tag->tag = "My Entity Name (Bobby)";
	attachComponent(entity, "Transform");
	attachComponent(entity, "Camera");

	ECS::Entity entity2 = createEntity();
	// --- For each component
	TagComponent* tag2 = (TagComponent*)attachComponent(entity2, "Tag");
	tag2->tag = "The Grindstone Entity B)";
	attachComponent(entity2, "Transform");

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
