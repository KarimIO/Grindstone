#pragma once

#include <string>
#include "Common/ResourcePipeline/Uuid.hpp"
#include "EngineCore/Assets/Asset.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBufferBinding;
		class TextureBinding;
		class UniformBuffer;
	}

	struct MaterialAsset : public Asset {
		MaterialAsset(Uuid uuid, std::string_view name, ShaderAsset& shader) : Asset(uuid, name), shaderAsset(shader) {}
		ShaderAsset& shaderAsset;
		GraphicsAPI::TextureBinding* textureBinding = nullptr;
		GraphicsAPI::UniformBufferBinding* uniformBufferBinding = nullptr;
		GraphicsAPI::UniformBuffer* uniformBufferObject = nullptr;
		char* buffer = nullptr;
		std::vector<std::pair<ECS::Entity, void*>> renderables;

		DEFINE_ASSET_TYPE("Material", AssetType::Material)
	};
}
