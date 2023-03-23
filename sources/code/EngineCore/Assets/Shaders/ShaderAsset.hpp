#pragma once

#include <string>
#include <vector>
#include "Common/Graphics/Pipeline.hpp"
#include "ShaderReflectionData.hpp"
#include "EngineCore/Assets/Materials/MaterialAsset.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct ShaderAsset : public Asset {
		ShaderAsset(Uuid uuid, std::string_view name, GraphicsAPI::Pipeline* pipeline) : Asset(uuid, name), pipeline(pipeline) {}
		ShaderReflectionData reflectionData;
		GraphicsAPI::Pipeline* pipeline = nullptr;
		GraphicsAPI::TextureBindingLayout* textureBindingLayout = nullptr;
		std::vector<Uuid> materials;

		DEFINE_ASSET_TYPE("Shader", AssetType::Shader)
	};
}
