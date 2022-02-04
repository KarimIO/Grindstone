#include "EngineCore/EngineCore.hpp"
#include "EngineCore/BuildSettings/SceneBuildSettings.hpp"
#include "SceneLoaderJson.hpp"
#include "SceneWriterJson.hpp"
#include "Manager.hpp"
using namespace Grindstone::SceneManagement;

void SceneManager::LoadDefaultScene() {
	BuildSettings::SceneBuildSettings settings; 
	const char* defaultPath = settings.GetDefaultScene();
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
	Scene* newScene = new Scene();
	SceneLoaderJson sceneLoader(newScene, path);
	CloseActiveScenes();
	scenes[path] = newScene;

	return newScene;
}

Scene* SceneManager::LoadSceneAdditively(const char* path) {
	Scene* newScene = new Scene();
	SceneLoaderJson sceneLoader(newScene, path);
	scenes[path] = newScene;

	return newScene;
}

void SceneManager::SaveScene(const char* path, Scene* scene) {
	SceneWriterJson sceneWriter(scene, path);
}

Scene* SceneManager::CreateEmptyScene(const char* name) {
	CloseActiveScenes();
	return nullptr;
}

Scene* SceneManager::AddEmptyScene(const char *name) {
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
