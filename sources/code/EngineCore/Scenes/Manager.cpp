#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/BuildSettings/SceneBuildSettings.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "SceneLoaderJson.hpp"
#include "SceneWriterJson.hpp"
#include "Manager.hpp"

using namespace Grindstone::SceneManagement;
using namespace Grindstone::Memory;

extern "C" {
	ENGINE_CORE_API Scene* SceneManagerGetActiveScene() {
		const SceneManager* sceneManager = EngineCore::GetInstance().GetSceneManager();
		if (sceneManager->scenes.empty()) {
			return nullptr;
		}

		Scene* scene = sceneManager->scenes.begin()->second;
		return scene;
	}
}

Grindstone::SceneManagement::SceneManager::~SceneManager() {
	CloseActiveScenes();
}

void SceneManager::LoadDefaultScene() {
	const BuildSettings::SceneBuildSettings settings;
	Grindstone::Uuid defaultSceneUuid = settings.GetDefaultScene();

	if (!defaultSceneUuid.IsValid()) {
		CreateEmptyScene("Untitled Scene");
		return;
	}

	LoadScene(defaultSceneUuid);
}

Scene* SceneManager::LoadScene(Grindstone::Uuid uuid) {
	CloseActiveScenes();
	return LoadSceneAdditively(uuid);
}

Scene* SceneManager::LoadSceneAdditively(Grindstone::Uuid uuid) {
	Scene* newScene = AllocatorCore::Allocate<Scene>();
	scenes[uuid] = newScene;
	SceneLoaderJson sceneLoader(newScene, uuid);
	ProcessSceneAfterLoading(newScene);

	return newScene;
}

void SceneManager::AddPostLoadProcess(std::function<void(Scene*)> fn) {
	postLoadProcesses.push_back(fn);
}

void SceneManager::ProcessSceneAfterLoading(Scene* scene) {
	for (std::function<void(Scene*)> postLoadProcess : postLoadProcesses) {
		postLoadProcess(scene);
	}
}

void SceneManager::SaveScene(const char* path, Scene* scene) {
	SceneWriterJson sceneWriter(scene, path);
}

Scene* SceneManager::CreateEmptyScene(const char* name) {
	CloseActiveScenes();
	return CreateEmptySceneAdditively(name);
}

Scene* SceneManager::CreateEmptySceneAdditively(const char *name) {
	Scene* newScene = AllocatorCore::Allocate<Scene>();
	scenes[name] = newScene;

	return newScene;
}

void SceneManager::CloseActiveScenes() {
	for (auto& scene : scenes) {
		AllocatorCore::Free(scene.second);
	}

	scenes.clear();
}
