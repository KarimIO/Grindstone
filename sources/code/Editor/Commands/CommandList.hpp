#pragma once

#include <string>
#include <vector>
#include <entt/entt.hpp>

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
				SceneManagement::Scene* scene,
				entt::entity entityId,
				const char* componentName
			);
			void SetUndoCount(size_t undoCount);
			void AddCommand(BaseCommand* command);
			bool Redo();
			bool Undo();
		private:
			std::vector<BaseCommand*> commands;
			size_t commandIterator = 0;
			size_t usedCommandCount = 0;
		};
	}
}
