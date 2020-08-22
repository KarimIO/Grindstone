#pragma once

#include <string>
#include <map>

#include "Scene.hpp"

namespace Grindstone {
	class SceneManager {
	public:
		std::unordered_map<std::string, Scene*> scenes_;
	};
}
