#include <stdexcept>
#include <iostream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Scene.hpp"

#include "EngineCore/CoreComponents/Mesh/MeshComponent.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dManager.hpp"

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
	EngineCore& engineCore = EngineCore::GetInstance();
	auto mesh3dManager = engineCore.mesh3dManager;

	auto meshAndMeshRendererView = registry.view<MeshComponent>();
	meshAndMeshRendererView.each([&](
		entt::entity entity,
		MeshComponent& meshComponent
	) {
		if (meshComponent.mesh == nullptr) {
			return;
		}

		mesh3dManager->DecrementMeshCount(ECS::Entity{ entity, this }, meshComponent.mesh->uuid);
	});

	registry.clear();
}

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
	return name;
}

const char* Scene::GetPath() {
	return path;
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
