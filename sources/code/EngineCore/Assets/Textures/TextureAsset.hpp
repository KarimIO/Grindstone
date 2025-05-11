#pragma once

#include "EngineCore/Assets/Asset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class Image;
		class Sampler;
	}

	struct TextureAsset : public Asset {
		TextureAsset(Uuid uuid) : Asset(uuid, uuid.ToString()) {}
		GraphicsAPI::Image* image = nullptr;
		GraphicsAPI::Sampler* defaultSampler = nullptr;

		DEFINE_ASSET_TYPE("Texture", AssetType::Texture)
	};
}
