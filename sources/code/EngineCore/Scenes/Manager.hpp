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
			virtual void SaveScene(const char* path, Scene* scene);
			virtual Scene* LoadSceneAdditively(const char* path);
			virtual Scene* LoadScene(const char* path);
			virtual Scene* CreateEmptyScene(const char* name);
			virtual Scene* AddEmptyScene(const char* name);
			std::unordered_map<std::string, Scene*> scenes;
		private:
			void CloseActiveScenes();	
		};
	}
}
