#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>

#include <Common/Buffer.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>

#include "TextureAsset.hpp"

namespace Grindstone {
	class TextureImporter : public SpecificAssetImporter<TextureAsset, AssetType::Texture> {
	public:
		virtual ~TextureImporter() override;

		virtual void* LoadAsset(Uuid uuid) override;
		virtual void QueueReloadAsset(Uuid uuid) override;
	private:
		std::map<std::string, TextureAsset> texturesByAddress;
	};
}
