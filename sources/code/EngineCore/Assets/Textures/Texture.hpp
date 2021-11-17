#pragma once

#include "Common/Graphics/Texture.hpp"
#include "../AssetFile.hpp"

namespace Grindstone {
	struct TextureAsset : public AssetFile {
		Grindstone::GraphicsAPI::Texture* texture;
	};
}
