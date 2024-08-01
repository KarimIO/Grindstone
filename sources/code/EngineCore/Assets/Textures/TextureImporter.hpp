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
		virtual void* ProcessLoadedFile(Uuid uuid) override;
		virtual void* ProcessLoadedFile(const char* path) override;
		virtual void* ProcessLoadedFile(Uuid uuid, std::string& assetName, Grindstone::Buffer buffer, TextureAsset& textureAsset);
		virtual void QueueReloadAsset(Uuid uuid) override;

		virtual bool TryGetIfLoaded(const char* path, void*& output) override;
	private:
		std::map<std::string, TextureAsset> texturesByPath;
	};
}
