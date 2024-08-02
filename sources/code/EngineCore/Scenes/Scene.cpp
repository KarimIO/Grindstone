#include <stdexcept>
#include <iostream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Parent/ParentComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Scene.hpp"
#include <EngineCore/Profiling.hpp>

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

extern "C" {
	ENGINE_CORE_API const char* SceneGetName(Scene* scene) {
		return scene->GetName();
	}

	ENGINE_CORE_API const char* SceneGetPath(Scene* scene) {
		return scene->GetPath();
	}

	ENGINE_CORE_API entt::entity SceneCreateEntity(Scene* scene) {
		return scene->CreateEntity().GetHandle();
	}

	ENGINE_CORE_API void SceneDestroyEntity(Scene* scene, entt::entity entityHandle) {
		scene->DestroyEntity((ECS::EntityHandle)entityHandle);
	}
}

Scene::~Scene() {
	auto view = GetEntityRegistry().view<entt::entity>();

	view.each(
		[&](entt::entity entity) {
			DestroyEntity(entity);
		}
	);
}

ECS::Entity Scene::CreateEmptyEntity(entt::entity entityToUse) {
	if (entityToUse == entt::null) {
		entt::entity entityId = GetEntityRegistry().create();
		return { entityId, this };
	}

	entt::entity entityId = GetEntityRegistry().create(entityToUse);
	return { entityId, this };
}

void Scene::DestroyEntity(ECS::EntityHandle entityId) {
	DestroyEntity(ECS::Entity(entityId, this));
}

void Scene::DestroyEntity(ECS::Entity entity) {
	GetComponentRegistrar()->DestroyEntity(entity);
}

ECS::Entity Scene::CreateEntity(entt::entity entityToUse) {
	auto entity = CreateEmptyEntity(entityToUse);
	entity.AddComponent<TagComponent>("New Entity");
	entity.AddComponent<TransformComponent>();
	entity.AddComponent<ParentComponent>();
	return entity;
}

const char* Scene::GetName() {
	return name.c_str();
}

const char* Scene::GetPath() {
	return path.string().c_str();
}

// Made so that entities can access componentregistrat without requiring enginecore.
ECS::ComponentRegistrar* Scene::GetComponentRegistrar() const {
	return EngineCore::GetInstance().GetComponentRegistrar();
}

entt::registry& Scene::GetEntityRegistry() {
	return EngineCore::GetInstance().GetEntityRegistry();
}
