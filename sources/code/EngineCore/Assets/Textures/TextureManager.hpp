#pragma once

#include <string>
#include <vector>
#include <map>

#include "Texture.hpp"

namespace Grindstone {
	class TextureManager {
		public:
			TextureAsset& LoadTexture(const char* path);
			bool TryGetTexture(const char* path, TextureAsset*& material);
		private:
			TextureAsset& CreateTextureFromFile(const char* path);
			TextureAsset CreateFromDds(const char* data, size_t fileSize);
			std::map<std::string, TextureAsset> textures;
	};
}
