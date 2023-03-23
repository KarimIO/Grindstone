#pragma once

#include "Common/Graphics/Texture.hpp"
#include "EngineCore/Assets/Asset.hpp"
using namespace Grindstone::GraphicsAPI;

namespace Grindstone {
	struct TextureAsset : public Asset {
		TextureAsset(Uuid uuid, std::string_view name, Texture* texture) : Asset(uuid, name), texture(texture) {}
		Texture* texture = nullptr;

		DEFINE_ASSET_TYPE("Texture", AssetType::Texture)
	};
}
