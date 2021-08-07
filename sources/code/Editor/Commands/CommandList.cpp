#include "EntityCommands.hpp"
#include "ComponentCommands.hpp"
#include "CommandList.hpp"

namespace Grindstone {
	namespace Editor {
		CommandList::CommandList() {
			const size_t undoCount = 64;
			commands.resize(undoCount);
		}

		void CommandList::AddNewEntity(SceneManagement::Scene* scene) {
			AddCommand(new AddEntityCommand(scene));
		}
	
		void CommandList::AddComponent(
			SceneManagement::Scene* scene,
			entt::entity entityId,
			const char* componentName
		) {
			AddCommand(new AddComponentCommand(scene, entityId, componentName));
		}

		void CommandList::SetUndoCount(size_t undoCount) {
			commands.resize(undoCount);

			// Remove old commands and move new commands
		}

		// TODO: Make it a ring buffer
		void CommandList::AddCommand(BaseCommand* command) {
			/*for (size_t i = commandIterator; i < usedCommandCount; ++i) {
				delete commands[i];
			}*/

			commands[commandIterator] = command;

			usedCommandCount += 1;
			commandIterator = (commandIterator + 1) % commands.size();
		}

		bool CommandList::Redo() {
			if (commandIterator == usedCommandCount) {
				return false;
			}

			commands[commandIterator]->Redo();
			commandIterator = (commandIterator + 1) % commands.size();
			return true;
		}

		bool CommandList::Undo() {
			if (usedCommandCount == 0) {
				return false;
			}

			commandIterator = (commandIterator - 1) % commands.size();
			commands[commandIterator]->Undo();
			return true;
		}
	}
}
