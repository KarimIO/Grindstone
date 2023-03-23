#pragma once

#include <string>
#include "Common/ResourcePipeline/Uuid.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBufferBinding;
		class TextureBinding;
		class UniformBuffer;
	}

	struct ShaderAsset;

	struct MaterialAsset : public Asset {
		MaterialAsset(Uuid uuid, std::string_view name, Uuid shaderUuid) : Asset(uuid, name), shaderUuid(shaderUuid) {}
		Uuid shaderUuid;
		GraphicsAPI::TextureBinding* textureBinding = nullptr;
		GraphicsAPI::UniformBufferBinding* uniformBufferBinding = nullptr;
		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		char* buffer = nullptr;
		std::vector<std::pair<ECS::Entity, void*>> renderables;

		DEFINE_ASSET_TYPE("Material", AssetType::Material)
	};
}
