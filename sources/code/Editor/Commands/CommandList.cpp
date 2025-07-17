#include "EntityCommands.hpp"
#include "ComponentCommands.hpp"
#include "CommandList.hpp"

namespace Grindstone {
	namespace Editor {
		CommandList::CommandList() {
			const size_t undoCount = 3;
			commands.resize(undoCount);
		}

		void CommandList::AddNewEntity(SceneManagement::Scene* scene) {
			AddCommand(new AddEntityCommand(scene));
		}

		void CommandList::AddComponent(
			ECS::Entity entity,
			Grindstone::HashedString componentName
		) {
			AddCommand(new AddComponentCommand(entity, componentName));
		}

		void CommandList::SetUndoCount(size_t undoCount) {
			commands.resize(undoCount);

			// Remove old commands and move new commands
		}

		void CommandList::AddCommand(BaseCommand* command) {
			size_t max = commands.size();

			for (size_t i = commandIndex; i != stackEndIndex; i = GetNextNumber(i)) {
				delete commands[i];
				commands[i] = nullptr;
			}

			if (usedCommandCount == max) {
				delete commands[commandIndex];
			}
			commands[commandIndex] = command;

			if (usedCommandCount >= max - 1) {
				stackBeginIndex = GetNextNumber(stackBeginIndex);
			}

			if (usedCommandCount < max) {
				++usedCommandCount;
			}

			commandIndex = GetNextNumber(commandIndex);
			stackEndIndex = commandIndex;

			canUndo = true;
			canRedo = false;

			auto ac = (AddEntityCommand*)commands[0];
			uint32_t a = ac ? (std::uint32_t)ac->entityId : 69;
			auto bc = (AddEntityCommand*)commands[1];
			uint32_t b = bc ? (std::uint32_t)bc->entityId : 69;
			auto cc = (AddEntityCommand*)commands[2];
			uint32_t c = cc ? (std::uint32_t)cc->entityId : 69;
		}

		bool CommandList::HasAvailableRedo() {
			return canRedo;
		}

		bool CommandList::HasAvailableUndo() {
			return canUndo;
		}

		bool CommandList::Redo() {
			if (!HasAvailableRedo()) {
				return false;
			}

			if (commandIndex == stackEndIndex) {
				canRedo = false;
			}
			canUndo = true;

			commands[commandIndex]->Redo();
			commandIndex = GetNextNumber(commandIndex);
			return true;
		}

		bool CommandList::Undo() {
			if (!HasAvailableUndo()) {
				return false;
			}

			if (commandIndex == stackBeginIndex) {
				canUndo = false;
			}
			canRedo = true;

			commandIndex = GetPreviousNumber(commandIndex);
			commands[commandIndex]->Undo();
			return true;
		}

		size_t CommandList::GetNextNumber(size_t originalNumber) {
			if (++(originalNumber) == commands.size()) {
				originalNumber = 0;
			}

			return originalNumber;
		}

		size_t CommandList::GetPreviousNumber(size_t originalNumber) {
			if (--(originalNumber) == -1) {
				originalNumber = commands.size() - 1;
			}

			return originalNumber;
		}
	}
}
