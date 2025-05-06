#pragma once

#include <string>
#include <vector>

#include <Common/Buffer.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/Asset.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/ECS/Entity.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class DescriptorSet;
		class Buffer;
	}

	struct MaterialAsset : public Asset {
		MaterialAsset(Grindstone::Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pipelineSetAsset;
		Grindstone::GraphicsAPI::DescriptorSet* materialDescriptorSet = nullptr;
		Grindstone::GraphicsAPI::Buffer* materialDataUniformBuffer = nullptr;
		std::vector<Grindstone::AssetReference<Grindstone::TextureAsset>> textures;
		Grindstone::Buffer materialDataBuffer;

		DEFINE_ASSET_TYPE("Material", AssetType::Material)
	};
}
