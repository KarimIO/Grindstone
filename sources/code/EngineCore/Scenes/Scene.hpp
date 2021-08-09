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
			virtual ECS::Entity CreateEmptyEntity();
			virtual ECS::Entity CreateEntity();
			virtual void DestroyEntity(ECS::EntityHandle entityId);
			virtual void DestroyEntity(ECS::Entity entity);
			virtual const char* GetName();
			virtual entt::registry& GetEntityRegistry();
			virtual ECS::ComponentRegistrar* GetComponentRegistrar();
			void Update();
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
