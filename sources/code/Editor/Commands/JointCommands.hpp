#pragma once

#include "CommandList.hpp"

namespace Grindstone {
	namespace Editor {
		class JointCommand : public BaseCommand {
		public:
			virtual void Redo() override;
			virtual void Undo() override;
			virtual ~JointCommand() {}
		};
	}
}
