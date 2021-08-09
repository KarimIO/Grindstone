#pragma once

#include <string>
#include <vector>
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;
	}

	namespace Editor {
		class BaseCommand {
		public:
			virtual void Redo() = 0;
			virtual void Undo() = 0;
			virtual ~BaseCommand() {}
		};
		
		class CommandList {
		public:
			CommandList();
			void AddNewEntity(SceneManagement::Scene* scene);
			void AddComponent(
				ECS::Entity entity,
				const char* componentName
			);
			void SetUndoCount(size_t undoCount);
			void AddCommand(BaseCommand* command);
			bool HasAvailableUndo();
			bool HasAvailableRedo();
			bool Redo();
			bool Undo();
		private:
			size_t GetNextNumber(size_t originalNumber);
			size_t GetPreviousNumber(size_t originalNumber);
		private:
			std::vector<BaseCommand*> commands;
			bool canUndo = false;
			bool canRedo = false;
			size_t commandIndex = 0;
			size_t stackBeginIndex = 0;
			size_t stackEndIndex = 0;
			size_t usedCommandCount = 0;
		};
	}
}
