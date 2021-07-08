#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/ECS/SystemRegistrar.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	class EngineCore;

	namespace ECS {
		class ComponentRegistrar;
	}

	namespace SceneManagement {
		class SceneLoaderJson;

		class Scene {
			friend SceneLoaderJson;
		public:
			Scene(EngineCore*, ECS::ComponentRegistrar*, ECS::SystemRegistrar*);
			virtual ECS::Entity createEntity();
			virtual ECS::Entity createDefaultEntity();
			virtual void* attachComponent(ECS::Entity entity, const char* componentName);
			virtual const char* getName();
			virtual entt::registry* getEntityRegistry();
			virtual ECS::ComponentRegistrar* getComponentRegistrar();
			void update();
		private:
			EngineCore* engineCore;
			ECS::SystemRegistrar* systemRegistrar;
			ECS::ComponentRegistrar* componentRegistrar;
			entt::registry registry;
			std::string name;
			std::string path;
		};
	}
}
