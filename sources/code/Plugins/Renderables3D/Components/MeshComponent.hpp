#pragma once

#include <string>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "../Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	struct MeshComponent {
		AssetReference<Mesh3dAsset> mesh;
		REFLECT("Mesh")
	};
}
