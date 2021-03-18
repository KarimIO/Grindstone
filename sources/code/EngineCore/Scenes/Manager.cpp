#include "EngineCore/EngineCore.hpp"
#include "EngineCore/BuildSettings/SceneBuildSettings.hpp"
#include "SceneLoaderJson.hpp"
#include "Manager.hpp"
using namespace Grindstone::SceneManagement;

SceneManager::SceneManager(EngineCore* core) : engineCore(core) {}

void SceneManager::loadDefaultScene() {
	BuildSettings::SceneBuildSettings settings; 
	const char* defaultPath = settings.getDefaultScene();
	loadScene(defaultPath);
}

void SceneManager::update() {
	for (auto& scene : scenes) {
		scene.second->update();
	}
}

Scene* SceneManager::loadScene(const char *path) {
	Scene* newScene = new Scene(engineCore->getComponentRegistrar(), engineCore->getSystemRegistrar());
	SceneLoaderJson sceneLoader(newScene, path);
	scenes[path] = newScene;

	return newScene;
}

Scene* SceneManager::addEmptyScene(const char *name) {
	Scene* newScene = new Scene(engineCore->getComponentRegistrar(), engineCore->getSystemRegistrar());
	scenes[name] = newScene;

	return newScene;
}
