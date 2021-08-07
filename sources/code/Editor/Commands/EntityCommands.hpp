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
		private:
			SceneManagement::Scene* scene;
			entt::entity entityId;
		};

		class DeleteEntityCommand : public BaseCommand {
		public:
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~DeleteEntityCommand() {}
		private:
			entt::registry& registry;
			entt::entity entityId;
			std::vector<AddComponentCommand> componentCommands;
		};
	}
}
