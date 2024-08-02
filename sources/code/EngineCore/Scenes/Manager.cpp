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
	const char* defaultPath = settings.GetDefaultScene();

	if (defaultPath == nullptr || strlen(defaultPath) == 0) {
		CreateEmptyScene("Untitled Scene");
		return;
	}

	LoadScene(defaultPath);
}

void SceneManager::EditorUpdate() {
	GRIND_PROFILE_SCOPE("SceneManager::EditorUpdate()");
	for (std::pair<const std::string, Scene*>& scene : scenes) {
		scene.second->EditorUpdate();
	}
}

void SceneManager::Update() {
	GRIND_PROFILE_SCOPE("SceneManager::Update()");
	for (std::pair<const std::string, Scene*>& scene : scenes) {
		scene.second->Update();
	}
}

Scene* SceneManager::LoadScene(const char* path) {
	CloseActiveScenes();
	return LoadSceneAdditively(path);
}

Scene* SceneManager::LoadSceneAdditively(const char* path) {
	Scene* newScene = AllocatorCore::Allocate<Scene>();
	scenes[path] = newScene;
	SceneLoaderJson sceneLoader(newScene, path);
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
