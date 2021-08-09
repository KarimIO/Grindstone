#pragma once

#include <entt/entt.hpp>
#include "CommandList.hpp"
#include "ComponentCommands.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace Editor {
		class AddEntityCommand : public BaseCommand {
		public:
			AddEntityCommand(SceneManagement::Scene* scene);
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~AddEntityCommand() {}
		public:
			SceneManagement::Scene* scene;
			ECS::EntityHandle entityId;
		};

		class DeleteEntityCommand : public BaseCommand {
		public:
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~DeleteEntityCommand() {}
		private:
			entt::registry& registry;
			ECS::EntityHandle entityId;
			std::vector<AddComponentCommand> componentCommands;
		};
	}
}
