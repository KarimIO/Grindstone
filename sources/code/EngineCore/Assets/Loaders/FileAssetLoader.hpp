#pragma once

#include <filesystem>

#include "AssetLoader.hpp"

namespace Grindstone {
	namespace Assets {
		class FileAssetLoader : public AssetLoader {
		public:
			virtual void Load(std::filesystem::path path, char*& outContents, size_t& fileSize) override;
			virtual void Load(Uuid uuid, char*& outContents, size_t& fileSize) override;
			virtual bool LoadText(Uuid uuid, std::string& outContents) override;
		};
	}
}
