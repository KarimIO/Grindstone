#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>

#include "../ECS/Entity.hpp"

namespace Grindstone {
	class Scene {
	public:
		Scene();
		ECS::Entity createEntity();
		bool attachComponent(ECS::Entity entity, const char* componentName);
		bool load(const char* path);
		bool loadFromText(const char* path);
		bool loadFromBinary(const char* path);
		virtual entt::registry* getEntityRegistry();
		void update();
	private:
		entt::registry registry;
		std::string name;
		std::string path;
	};
}
