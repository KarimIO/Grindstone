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
		
		Grindstone::GraphicsAPI::ComputePipeline* pipeline = nullptr;
		Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = nullptr;

		Grindstone::GraphicsAPI::ComputePipeline* GetPipeline() const {
			return pipeline;
		}

		Grindstone::GraphicsAPI::PipelineLayout* GetPipelineLayout() const {
			return pipelineLayout;
		}

		DEFINE_ASSET_TYPE("Compute PipelineSet", AssetType::ComputePipelineSet)
	};
}
