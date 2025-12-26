#pragma once

#include <EngineCore/WorldContext/WorldContextSet.hpp>

namespace Grindstone {
	namespace Scripting {
		namespace CSharp {
			void UpdateSystem(Grindstone::WorldContextSet& worldContextSet);
			void UpdateEditorSystem(Grindstone::WorldContextSet& worldContextSet);
		}
	}
}
