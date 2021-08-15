#pragma once

#include <filesystem>
#include <string>
#include <map>

#include "Material.hpp"

namespace Grindstone {
	class BaseAssetRenderer;

	class MaterialManager {
		public:
			Material& LoadMaterial(BaseAssetRenderer* assetRenderer, const char* path);
			bool TryGetMaterial(const char* path, Material*& material);
		private:
			Material& CreateMaterialFromFile(BaseAssetRenderer* assetRenderer, const char* path);
			void CreateMaterialFromData(
				std::filesystem::path relativePath,
				Material& material,
				BaseAssetRenderer* assetRenderer,
				const char* data
			);
		private:
			std::map<std::string, Material> materials;
	};
}
