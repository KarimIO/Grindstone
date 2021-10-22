#pragma once

#include <string>

namespace Grindstone {
	namespace Importers {
		class MaterialImporter : public Importer {
		public:
			void Import(std::filesystem::path& path) override;
		};

		void ImportMaterial(std::filesystem::path& inputPath);

		struct StandardMaterialCreateInfo {
			std::string materialName = "";
			std::string albedoPath = "";
			std::string specularPath = "";
			std::string normalPath = "";
			std::string roughnessPath = "";
			float albedoColor[4];
			float metalness;
			float roughness;
		};

		bool CreateStandardMaterial(StandardMaterialCreateInfo ci, std::filesystem::path path);
		bool CreateCutoutMaterial(StandardMaterialCreateInfo ci, std::filesystem::path path);
	}
}
