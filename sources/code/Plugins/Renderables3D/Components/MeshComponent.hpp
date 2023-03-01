#pragma once

#include <string>
#include <vector>
#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "../Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	struct MeshComponent {
		AssetReference<ShaderAsset> mesh;
		REFLECT("Mesh")
	};
}
