#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace ECS {
		class ComponentRegistrar;
	}

	namespace SceneManagement {
		class SceneLoaderJson;

		class Scene {
			friend SceneLoaderJson;
		public:
			Scene(ECS::ComponentRegistrar*, ECS::SystemRegistrar*);
			ECS::Entity createEntity();
			virtual const char* getName();
			virtual entt::registry* getEntityRegistry();
			virtual ECS::ComponentRegistrar* getComponentRegistrar();
			void update();
		private:
			ECS::SystemRegistrar* systemRegistrar;
			ECS::ComponentRegistrar* componentRegistrar;
			entt::registry registry;
			std::string name;
			std::string path;
		};
	}
}
