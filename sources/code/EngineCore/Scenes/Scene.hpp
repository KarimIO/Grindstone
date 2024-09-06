#pragma once

#include <string>
#include <string_view>
#include <filesystem>
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
		private:
			std::string name;
			std::filesystem::path path;
		public:
			~Scene();
			virtual Grindstone::ECS::Entity CreateEmptyEntity(entt::entity entityToUse = entt::null);
			virtual Grindstone::ECS::Entity CreateEntity(entt::entity entityToUse = entt::null);
			virtual void DestroyEntity(Grindstone::ECS::EntityHandle entityId);
			virtual void DestroyEntity(Grindstone::ECS::Entity entity);
			virtual void SetName(std::string_view);
			virtual const std::string& GetName() const;
			virtual const std::filesystem::path& GetPath() const;
			virtual bool HasPath() const;
			virtual ECS::ComponentRegistrar* GetComponentRegistrar() const;
			virtual entt::registry& GetEntityRegistry();
		};
	}
}
