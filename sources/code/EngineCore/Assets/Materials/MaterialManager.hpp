#pragma once

#include <string>
#include <vector>
#include <map>

#include "Material.hpp"

namespace Grindstone {
	class MaterialManager {
		public:
			Material& LoadMaterial(const char* path);
			bool TryGetMaterial(const char* path, Material*& material);
		private:
			Material& CreateMaterialFromFile(const char* path);
		private:
			std::map<std::string, Material> materials;
	};
}
