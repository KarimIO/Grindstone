#pragma once

#include <string>
#include "Common/ResourcePipeline/Uuid.hpp"

namespace Grindstone {
	namespace Importers {
		class MaterialImporter : public Importer {
		public:
			void Import(std::filesystem::path& path) override;
			Uuid GetUuidAfterImport() const;
		private:
			Uuid uuid;
		};

		Uuid ImportMaterial(std::filesystem::path& inputPath);

		struct StandardMaterialCreateInfo {
			std::string materialName = "";
			std::filesystem::path albedoPath;
			std::filesystem::path specularPath;
			std::filesystem::path normalPath;
			std::filesystem::path roughnessPath;
			float albedoColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			float metalness = 0.0f;
			float roughness = 0.5f;
		};

		void CreateStandardMaterial(StandardMaterialCreateInfo& ci, std::filesystem::path path);
		void CreateCutoutMaterial(StandardMaterialCreateInfo& ci, std::filesystem::path path);
	}
}
