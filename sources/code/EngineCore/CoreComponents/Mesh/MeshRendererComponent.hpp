#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	struct MeshRendererComponent {
		std::vector<int> materialPaths;

		REFLECT("MeshRenderer")
	};
}
