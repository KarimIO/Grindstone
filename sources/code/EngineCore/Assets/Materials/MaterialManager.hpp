#pragma once

#include <filesystem>
#include <string>
#include <map>

#include "Material.hpp"

namespace Grindstone {
	class MaterialManager {
		public:
			Material& LoadMaterial(const char* path);
			bool TryGetMaterial(const char* path, Material*& material);
		private:
			Material& CreateMaterialFromFile(const char* path);
			Material CreateMaterialFromData(std::filesystem::path relativePath, const char* data);
		private:
			std::map<std::string, Material> materials;
	};
}
