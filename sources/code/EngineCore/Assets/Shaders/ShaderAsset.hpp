#pragma once

#include <string>
#include <vector>
#include "Common/Graphics/Pipeline.hpp"
#include "ShaderReflectionData.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct Material;
	struct ShaderAsset : public Asset {
		ShaderReflectionData reflectionData;

		DEFINE_ASSET_TYPE
	};
}
