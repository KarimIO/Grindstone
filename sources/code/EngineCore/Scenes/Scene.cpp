#include <stdexcept>
#include <iostream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

ECS::Entity Scene::CreateEmptyEntity(entt::entity entityToUse) {
	if (entityToUse == entt::null) {
		entt::entity entityId = registry.create();
		return { entityId, this };
	}

	entt::entity entityId = registry.create(entityToUse);
	return { entityId, this };
}

void Scene::DestroyEntity(ECS::EntityHandle entityId) {
	registry.destroy(entityId);
}

void Scene::DestroyEntity(ECS::Entity entity) {
	DestroyEntity(entity.GetHandle());
}

ECS::Entity Scene::CreateEntity(entt::entity entityToUse) {
	auto entity = CreateEmptyEntity(entityToUse);
	entity.AddComponent<TagComponent>("New Entity");
	entity.AddComponent<TransformComponent>();
	return entity;
}

const char* Scene::GetName() {
	return name.c_str();
}

const char* Scene::GetPath() {
	return path.c_str();
}

// Made so that entities can access componentregistrat without requiring enginecore.
ECS::ComponentRegistrar* Scene::GetComponentRegistrar() const {
	return EngineCore::GetInstance().GetComponentRegistrar();
}

entt::registry& Scene::GetEntityRegistry() {
	return registry;
}

void Scene::Update() {
	EngineCore::GetInstance().GetSystemRegistrar()->Update(registry);
}

void Scene::EditorUpdate() {
	EngineCore::GetInstance().GetSystemRegistrar()->EditorUpdate(registry);
}
