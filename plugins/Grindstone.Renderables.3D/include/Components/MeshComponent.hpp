#pragma once

#include <string>
#include <vector>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "../Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	struct MeshComponent {
		AssetReference<Mesh3dAsset> mesh;
		REFLECT("Mesh")
	};
}
