#include <stdexcept>
#include <iostream>

#include "Scene.hpp"
#include <EngineCore/ECS/Core.hpp>

using namespace Grindstone;

Scene::Scene() : ecs_(this) {
}

void Scene::loadFromText(const char* path) {
	path_ = path;

	// Get from text
	name_ = path;

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
	ECS::Entity entity = ecs_.createEntity();
	// --- For each component
	ecs_.createComponent(entity, "Transform");

	/*
	ecs_.createComponent("Rotation");
	ecs_.createComponent("Scale");
	ecs_.createComponent("World");
	ecs_.createComponent("Hierarchy");
	ecs_.createComponent("Hierarchy");
	*/
	// --- 

}

void Scene::loadFromBinary(const char* path) {
	throw std::runtime_error("Scene::loadFromBinary is not implemented!");
}