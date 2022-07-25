#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct MeshComponent {
		MeshReference mesh;

		REFLECT("Mesh")
	};
}
