#pragma once

#include <string>
#include <map>

#include "EngineCore/ECS/ComponentRegistrar.hpp"
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
			virtual Grindstone::ECS::Entity CreateEmptyEntity(entt::entity entityToUse = entt::null);
			virtual Grindstone::ECS::Entity CreateEntity(entt::entity entityToUse = entt::null);
			virtual void DestroyEntity(Grindstone::ECS::EntityHandle entityId);
			virtual void DestroyEntity(Grindstone::ECS::Entity entity);
			virtual const char* GetName();
			virtual const char* GetPath();
			virtual ECS::ComponentRegistrar* GetComponentRegistrar() const;
			virtual entt::registry& GetEntityRegistry();
			void Update();
			void EditorUpdate();
		private:
			entt::registry registry;
			std::string name;
			std::string path;
		};
	}
}
