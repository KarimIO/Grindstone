#pragma once

#include <string>
#include <vector>
#include "Common/Graphics/Pipeline.hpp"
#include "ShaderReflectionData.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct Material;
	struct ShaderAsset : public Asset {
		ShaderAsset(Uuid uuid, std::string_view name) : Asset(uuid, name) {}
		ShaderReflectionData reflectionData;
		GraphicsAPI::Pipeline* pipeline = nullptr;
		GraphicsAPI::TextureBindingLayout* textureBindingLayout = nullptr;

		DEFINE_ASSET_TYPE
	};
}
