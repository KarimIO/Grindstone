#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"

namespace Grindstone {
	struct Mesh3d;

	struct MeshComponent {
		Mesh3d* mesh;
		std::string meshPath;

		REFLECT("Mesh")
	};
}
