#pragma once

#include <string>
#include <map>

#include <EngineCore/ECS/Controller.hpp>

namespace Grindstone {
	class Scene {
	public:
		Scene();
		void registerAll();
		void registerComponentArray();
		void loadFromText(const char* path);
		void loadFromBinary(const char* path);
		ECS::Controller ecs_;
	private:
		std::string name_;
		std::string path_;
	};
}
