#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>

#include "TextureAsset.hpp"
#include "EngineCore/Assets/AssetImporter.hpp"

namespace Grindstone {
	class TextureImporter : public AssetImporter {
	public:
		virtual void* ProcessLoadedFile(Uuid uuid) override;
		virtual void* ProcessLoadedFile(const char* path) override;
		virtual void* ProcessLoadedFile(Uuid uuid, const char* fileContents, size_t fileSize, TextureAsset& textureAsset);
		virtual void QueueReloadAsset(Uuid uuid) override;

		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
		virtual bool TryGetIfLoaded(const char* path, void*& output) override;
	private:
		std::map<Uuid, TextureAsset> textures;
		std::map<std::string, TextureAsset> texturesByPath;
	};
}
