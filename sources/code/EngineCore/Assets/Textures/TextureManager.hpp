#pragma once

#include <string>
#include <vector>
#include <map>

#include "Texture.hpp"

namespace Grindstone {
	class TextureManager {
		public:
			virtual TextureAsset& LoadTexture(const char* path);
			virtual void ReloadTextureIfLoaded(const char* path);
			TextureAsset& GetDefaultTexture();
		private:
			bool TryGetTexture(std::string& path, TextureAsset*& material);
			void LoadTextureFromFile(bool isReloading, std::string& path, TextureAsset& textureAsset);
			TextureAsset& CreateNewTextureFromFile(std::string& path);
			void CreateFromDds(bool isReloading, TextureAsset&, const char* fileName, const char* data, size_t fileSize);
			
			std::map<std::string, TextureAsset> textures;
			TextureAsset errorTexture;
	};
}
