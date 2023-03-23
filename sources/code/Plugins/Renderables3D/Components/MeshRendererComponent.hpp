#pragma once

#include <string>
#include <vector>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Materials/MaterialAsset.hpp"

namespace Grindstone {
	class EngineCore;

	struct MeshRendererComponent {
		std::vector<AssetReference<MaterialAsset>> materials;

		REFLECT("MeshRenderer")
	};
}
