#pragma once

#include <string>
#include <vector>

#include <Common/HashedString.hpp>
#include <EngineCore/Assets/Asset.hpp>
#include <EngineCore/Assets/Textures/TextureAsset.hpp>

#include "PipelineAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class ComputePipeline;
		class DescriptorSetLayout;
	}

	struct ComputePipelineAsset : public Asset {
		ComputePipelineAsset(Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		
		std::array<GraphicsAPI::DescriptorSetLayout*, 16> descriptorSetLayouts = {};
		Grindstone::GraphicsAPI::ComputePipeline* pipeline = nullptr;

		Grindstone::GraphicsAPI::ComputePipeline* GetPipeline() {
			return pipeline;
		}


		DEFINE_ASSET_TYPE("Compute PipelineSet", AssetType::ComputePipelineSet)
	};
}
