#pragma once

#include <string>
#include <vector>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Materials/MaterialAsset.hpp"

namespace Grindstone {
	struct MeshRendererComponent {
		std::vector<AssetReference<MaterialAsset>> materials;

		REFLECT("MeshRenderer")
	};
}
