#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	struct MeshComponent {
		std::string meshPath;

		REFLECT("Mesh")
	};
}
