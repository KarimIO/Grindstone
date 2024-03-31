#pragma once

#include <filesystem>

namespace Grindstone::Editor::Importers {
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
