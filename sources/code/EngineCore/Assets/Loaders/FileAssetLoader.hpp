#pragma once

#include "AssetLoader.hpp"

namespace Grindstone {
	namespace Assets {
		class FileAssetLoader : public AssetLoader {
		public:
			virtual void Load(Uuid uuid, std::vector<char>& outContents) override;
		};
	}
}
