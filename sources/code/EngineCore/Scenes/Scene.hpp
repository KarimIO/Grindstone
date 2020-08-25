#pragma once

#include <string>
#include <map>

#include <EngineCore/ECS/Controller.hpp>

namespace Grindstone {
	class Scene {
	public:
		Scene();
		bool load(const char* path);
		bool loadFromText(const char* path);
		bool loadFromBinary(const char* path);
		virtual ECS::Controller* getECS();
		void update();
	private:
		ECS::Controller ecs_;
		std::string name_;
		std::string path_;
	};
}
