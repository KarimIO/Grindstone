#include <stdexcept>
#include <iostream>

#include <EngineCore/Profiling.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

extern "C" {
	ENGINE_CORE_API const char* SceneGetName(Scene* scene) {
		return scene->GetName().c_str();
	}

	ENGINE_CORE_API const char* SceneGetPath(Scene* scene) {
		return scene->GetPath().string().c_str();
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
	entt::registry& registry = GetEntityRegistry();
	if (entityToUse == entt::null) {
		entt::entity entityId = registry.create();
		return { entityId, this };
	}

	if (registry.valid(entityToUse)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Registry already has entity with ID {}", static_cast<uint32_t>(entityToUse));
		entt::entity entityId = registry.create();
		return { entityId, this };
	}

	entt::entity entityId = registry.create(entityToUse);
	return { entityId, this };
}

void Scene::DestroyEntity(ECS::EntityHandle entityId) {
	DestroyEntity(ECS::Entity(entityId, this));
}

void Scene::DestroyEntity(ECS::Entity entity) {
	GetComponentRegistrar()->DestroyEntity(entity);
}

void Scene::SetName(std::string_view name) {
	this->name = name;
}

ECS::Entity Scene::CreateEntity(entt::entity entityToUse) {
	auto entity = CreateEmptyEntity(entityToUse);
	entity.AddComponent<TagComponent>("New Entity");
	entity.AddComponent<TransformComponent>();
	entity.AddComponent<ParentComponent>();
	return entity;
}

const std::string& Scene::GetName() const {
	return name;
}

const std::filesystem::path& Scene::GetPath() const {
	return path;
}

bool Scene::HasPath() const {
	return !path.empty();
}

// Made so that entities can access componentregistrat without requiring enginecore.
ECS::ComponentRegistrar* Scene::GetComponentRegistrar() const {
	return EngineCore::GetInstance().GetComponentRegistrar();
}

entt::registry& Scene::GetEntityRegistry() {
	return EngineCore::GetInstance().GetEntityRegistry();
}
