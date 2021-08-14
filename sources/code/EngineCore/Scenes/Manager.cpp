#include "EngineCore/EngineCore.hpp"
#include "EngineCore/BuildSettings/SceneBuildSettings.hpp"
#include "SceneLoaderJson.hpp"
#include "Manager.hpp"
using namespace Grindstone::SceneManagement;

void SceneManager::LoadDefaultScene() {
	BuildSettings::SceneBuildSettings settings; 
	const char* defaultPath = settings.getDefaultScene();
	LoadScene(defaultPath);
}

void SceneManager::Update() {
	for (auto& scene : scenes) {
		scene.second->Update();
	}
}

Scene* SceneManager::LoadScene(const char *path) {
	Scene* newScene = new Scene();
	SceneLoaderJson sceneLoader(newScene, path);
	scenes[path] = newScene;

	return newScene;
}

Scene* SceneManager::AddEmptyScene(const char *name) {
	Scene* newScene = new Scene();
	scenes[name] = newScene;

	return newScene;
}
