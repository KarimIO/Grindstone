#pragma once

#include <string>
#include <map>

#include "Scene.hpp"

namespace Grindstone {
	class EngineCore;

	namespace SceneManagement {
		class SceneManager {
		public:
			SceneManager(EngineCore* core);
			void loadDefaultScene();
			void update();
			virtual Scene* loadScene(const char *path);
			virtual Scene* addEmptyScene(const char *name);
			std::unordered_map<std::string, Scene*> scenes;
		private:
			EngineCore *engineCore;
		};
	}
}
