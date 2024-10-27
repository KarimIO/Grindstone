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

		virtual void* ProcessLoadedFile(Uuid uuid) override;
		virtual void* ProcessLoadedFile(std::string_view address) override;
		virtual void* ProcessLoadedFile(Uuid uuid, std::string& assetName, Grindstone::Buffer buffer, TextureAsset& textureAsset);
		virtual void QueueReloadAsset(Uuid uuid) override;

		virtual bool TryGetIfLoaded(std::string_view address, void*& output) override;
	private:
		std::map<std::string, TextureAsset> texturesByAddress;
	};
}
