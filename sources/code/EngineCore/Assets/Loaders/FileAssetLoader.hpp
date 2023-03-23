#pragma once

#include "AssetLoader.hpp"

namespace Grindstone {
	namespace Assets {
		class FileAssetLoader : public AssetLoader {
		public:
			virtual void Load(Uuid uuid, char*& outContents, size_t& fileSize) override;
			virtual bool LoadText(Uuid uuid, std::string& outContents) override;
		};
	}
}
