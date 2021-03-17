#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>

#include "../ECS/Entity.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;

		class SceneLoaderJson {
		public:
			SceneLoaderJson(Scene*, const char* path);
		private:
			ECS::Entity createEntity();
			bool attachComponent(ECS::Entity entity, const char* componentName);
			bool load(const char* path);
		private:
			Scene* scene;
			const char* path;
		};
	}
}
