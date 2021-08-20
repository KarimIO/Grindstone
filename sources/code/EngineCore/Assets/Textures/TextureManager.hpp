#pragma once

#include <string>
#include <vector>
#include <map>

#include "Texture.hpp"

namespace Grindstone {
	class TextureManager {
		public:
			virtual TextureAsset& LoadTexture(const char* path);
		private:
			bool TryGetTexture(const char* path, TextureAsset*& material);
			TextureAsset& CreateTextureFromFile(const char* path);
			TextureAsset CreateFromDds(const char* filenName, const char* data, size_t fileSize);
			std::map<std::string, TextureAsset> textures;
	};
}
