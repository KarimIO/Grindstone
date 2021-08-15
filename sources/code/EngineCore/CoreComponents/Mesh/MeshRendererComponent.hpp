#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	struct MeshRendererComponent {
		std::vector<std::string> materialPaths;

		REFLECT("Camera")
	};
}
