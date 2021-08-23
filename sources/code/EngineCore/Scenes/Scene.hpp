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
			Scene() = default;
			virtual ECS::Entity CreateEmptyEntity(entt::entity entityToUse = entt::null);
			virtual ECS::Entity CreateEntity(entt::entity entityToUse = entt::null);
			virtual void DestroyEntity(ECS::EntityHandle entityId);
			virtual void DestroyEntity(ECS::Entity entity);
			virtual const char* GetName();
			virtual ECS::ComponentRegistrar* GetComponentRegistrar() const;
			virtual entt::registry& GetEntityRegistry();
			void Update();
		private:
			entt::registry registry;
			std::string name;
			std::string path;
		};
	}
}
