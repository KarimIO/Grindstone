#pragma once

#include <string>
#include <vector>
#include "ShaderReflectionData.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsPipeline;
		class DescriptorSetLayout;
	}

	struct ShaderAsset : public Asset {
		ShaderAsset(Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		ShaderAsset(Uuid uuid, std::string_view name, GraphicsAPI::GraphicsPipeline* pipeline) : Asset(uuid, name), pipeline(pipeline) {}
		ShaderReflectionData reflectionData;
		uint8_t renderQueue = UINT8_MAX;
		GraphicsAPI::GraphicsPipeline* pipeline = nullptr;
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;

		DEFINE_ASSET_TYPE("Shader", AssetType::Shader)
	};
}
