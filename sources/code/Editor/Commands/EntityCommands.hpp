#pragma once

#include <entt/entt.hpp>
#include "CommandList.hpp"
#include "ComponentCommands.hpp"

namespace Grindstone {
	class WorldContextSet;

	namespace Editor {
		class AddEntityCommand : public BaseCommand {
		public:
			AddEntityCommand(Grindstone::WorldContextSet* cxtSet);
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~AddEntityCommand() {}
		public:
			Grindstone::WorldContextSet* cxtSet;
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
