#pragma once

#include <string>
#include <map>

#include "Scene.hpp"

namespace Grindstone {
	class EngineCore;

	namespace SceneManagement {
		class SceneManager {
		public:
			void LoadDefaultScene();
			void EditorUpdate();
			void Update();

			virtual void AddPostLoadProcess(std::function<void(Scene*)>);
			virtual void SaveScene(const char* path, Scene* scene);
			virtual Scene* LoadSceneAdditively(const char* path);
			virtual Scene* LoadScene(const char* path);
			virtual Scene* CreateEmptyScene(const char* name);
			virtual Scene* CreateEmptySceneAdditively(const char* name);
			std::unordered_map<std::string, Scene*> scenes;
		private:
			void CloseActiveScenes();

			void ProcessSceneAfterLoading(Scene* scene);
			std::vector<std::function<void(Scene*)>> postLoadProcesses;
		};
	}
}
