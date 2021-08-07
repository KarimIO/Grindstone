#pragma once

#include "CommandList.hpp"

namespace Grindstone {
	namespace Editor {
		class SetDataCommand : public BaseCommand {
		public:
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~SetDataCommand() {}
		};
	}
}
