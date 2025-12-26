#pragma once

#include <EngineCore/WorldContext/WorldContextSet.hpp>

namespace Grindstone {
	class EngineCore;

	namespace ECS {
		using SystemFactory = void(*)(Grindstone::WorldContextSet&);
	}
}
