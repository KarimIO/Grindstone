#pragma once

#include "Common/Graphics/Texture.hpp"
#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	struct TextureAsset : public Asset {
		Grindstone::GraphicsAPI::Texture* texture;
	};
}
