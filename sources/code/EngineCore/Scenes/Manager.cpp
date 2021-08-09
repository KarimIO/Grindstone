#include "EngineCore/EngineCore.hpp"
#include "EngineCore/BuildSettings/SceneBuildSettings.hpp"
#include "SceneLoaderJson.hpp"
#include "Manager.hpp"
using namespace Grindstone::SceneManagement;

void SceneManager::loadDefaultScene() {
	BuildSettings::SceneBuildSettings settings; 
	const char* defaultPath = settings.getDefaultScene();
	loadScene(defaultPath);
}

void SceneManager::update() {
	for (auto& scene : scenes) {
		scene.second->Update();
	}
}

Scene* SceneManager::loadScene(const char *path) {
	Scene* newScene = new Scene();
	SceneLoaderJson sceneLoader(newScene, path);
	scenes[path] = newScene;

	return newScene;
}

Scene* SceneManager::addEmptyScene(const char *name) {
	Scene* newScene = new Scene();
	scenes[name] = newScene;

	return newScene;
}
