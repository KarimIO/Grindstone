#pragma once

#include "CommandList.hpp"

namespace Grindstone {
	namespace Editor {
		class AddComponentCommand : public BaseCommand {
		public:
			AddComponentCommand(
				SceneManagement::Scene* scene,
				entt::entity entityId,
				const char* componentName
			);
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~AddComponentCommand() {}
		private:
			SceneManagement::Scene* scene;
			entt::entity entityId;
			std::string componentName;
		};

		class DeleteComponentCommand : public BaseCommand {
		public:
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~DeleteComponentCommand() {}
		private:
		};
	}
}
