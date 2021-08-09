#pragma once

#include "CommandList.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace Editor {
		class AddComponentCommand : public BaseCommand {
		public:
			AddComponentCommand(
				ECS::Entity entity,
				const char* componentName
			);
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~AddComponentCommand() {}
		private:
			ECS::Entity entity;
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
