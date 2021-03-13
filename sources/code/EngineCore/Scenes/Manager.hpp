#pragma once

#include <string>
#include <map>

#include "Scene.hpp"

namespace Grindstone {
	class EngineCore;

	class SceneManager {
	public:
		SceneManager(EngineCore* core);
		virtual Scene* loadScene(const char *path);
		virtual Scene* addEmptyScene(const char *name);
		std::unordered_map<std::string, Scene*> scenes;
	private:
		EngineCore *engineCore;
	};
}
