#pragma once

#include <string>
#include <map>

#include "Scene.hpp"

namespace Grindstone::SceneManagement {
	class SceneManager {
	public:
		~SceneManager();

		void LoadDefaultScene();
		void EditorUpdate();
		void Update();

		virtual void AddPostLoadProcess(std::function<void(Scene*)>);
		virtual void SaveScene(const char* path, Scene* scene);
		virtual Scene* LoadSceneAdditively(const char* path);
		virtual Scene* LoadScene(const char* path);
		virtual Scene* CreateEmptyScene(const char* name);
		virtual Scene* CreateEmptySceneAdditively(const char* name);
		virtual void CloseActiveScenes();
		std::unordered_map<std::string, Scene*> scenes;
	private:

		void ProcessSceneAfterLoading(Scene* scene);
		std::vector<std::function<void(Scene*)>> postLoadProcesses;
	};
}
