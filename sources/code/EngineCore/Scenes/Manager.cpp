#include "EngineCore/EngineCore.hpp"
#include "EngineCore/BuildSettings/SceneBuildSettings.hpp"
#include "SceneLoaderJson.hpp"
#include "SceneWriterJson.hpp"
#include "Manager.hpp"
using namespace Grindstone::SceneManagement;

extern "C" {
	ENGINE_CORE_API Scene* SceneManagerGetActiveScene() {
		SceneManager* sceneManager = EngineCore::GetInstance().GetSceneManager();
		if (sceneManager->scenes.empty()) {
			return nullptr;
		}

		Scene* scene = sceneManager->scenes.begin()->second;
		return scene;
	}
}

void SceneManager::LoadDefaultScene() {
	BuildSettings::SceneBuildSettings settings; 
	const char* defaultPath = settings.GetDefaultScene();

	if (defaultPath == nullptr || strlen(defaultPath) == 0) {
		CreateEmptyScene("Untitled Scene");
		return;
	}

	LoadScene(defaultPath);
}

void SceneManager::EditorUpdate() {
	for (auto& scene : scenes) {
		scene.second->EditorUpdate();
	}
}

void SceneManager::Update() {
	for (auto& scene : scenes) {
		scene.second->Update();
	}
}

Scene* SceneManager::LoadScene(const char* path) {
	CloseActiveScenes();
	return LoadSceneAdditively(path);
}

Scene* SceneManager::LoadSceneAdditively(const char* path) {
	Scene* newScene = new Scene();
	scenes[path] = newScene;
	std::string filepath = Grindstone::EngineCore::GetInstance().GetAssetPath(path).string();
	SceneLoaderJson sceneLoader(newScene, filepath.c_str());
	ProcessSceneAfterLoading(newScene);

	return newScene;
}

void SceneManager::AddPostLoadProcess(std::function<void(Scene*)> fn) {
	postLoadProcesses.push_back(fn);
}

void SceneManager::ProcessSceneAfterLoading(Scene* scene) {
	for (auto fn : postLoadProcesses) {
		fn(scene);
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
	Scene* newScene = new Scene();
	scenes[name] = newScene;

	return newScene;
}

void SceneManager::CloseActiveScenes() {
	for (auto& scene : scenes) {
		delete scene.second;
	}

	scenes.clear();
}
