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
		
		struct MetaData {
			std::vector<Grindstone::PipelineAssetMetaData::Buffer> buffers;
			std::vector<Grindstone::PipelineAssetMetaData::TextureSlot> textures;
		};

		MetaData metaData;
		GraphicsAPI::ComputePipeline* pipeline = nullptr;
		std::array<GraphicsAPI::DescriptorSetLayout*, 4> descriptorSetLayouts;

		const Grindstone::ComputePipelineAsset::MetaData* GetMetaData() const {
			return &metaData;
		}

		DEFINE_ASSET_TYPE("Compute PipelineSet", AssetType::ComputePipelineSet)
	};
}
