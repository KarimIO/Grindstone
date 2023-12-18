#pragma once

#include <string>
#include "Common/ResourcePipeline/Uuid.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DescriptorSet;
		class UniformBuffer;
	}

	struct ShaderAsset;

	struct MaterialAsset : public Asset {
		MaterialAsset(Uuid uuid, std::string_view name, Uuid shaderUuid) : Asset(uuid, name), shaderUuid(shaderUuid) {}
		Uuid shaderUuid;
		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		GraphicsAPI::DescriptorSet* descriptorSet = nullptr;
		char* buffer = nullptr;

		DEFINE_ASSET_TYPE("Material", AssetType::Material)
	};
}
