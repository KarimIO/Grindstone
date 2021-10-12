#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "Editor/EditorManager.hpp"
#include "MaterialCreator.hpp"
#include "EngineCore/Utils/Utilities.hpp"

using namespace Grindstone;
using namespace Grindstone::Converters;

bool CreateStandardOrCutoutMaterial(Materials::StandardMaterialCreateInfo createInfo, std::string path, bool isCutout) {
	std::filesystem::path p = path;
	std::filesystem::path pathToShader = std::filesystem::relative("../assets/test", p.parent_path());

	std::ofstream output(path);

	output << "{\n";
	
	output << "\t\"name\": \"" << createInfo.materialName << "\",\n";

	if (isCutout) {
		output << "\t\"shader\": \"cutout\",\n";
	}
	else {
		output << "\t\"shader\": \"" << Utils::FixStringSlashesReturn(pathToShader.string()) << "\",\n";
	}

	{
		output << "\t\"parameters\": {\n";
		output << "\t\t\"color\": [1, 1, 1, 1]\n";
		output << "\t},\n";
	}
	
	{
		output << "\t\"samplers\": {\n";

		std::filesystem::path albedo = std::filesystem::relative(createInfo.albedoPath, p.parent_path());
		output << "\t\t\"albedoTexture\": \"" << Utils::FixStringSlashesReturn(albedo.string()) << "\",\n";

		std::filesystem::path normal = std::filesystem::relative(createInfo.normalPath, p.parent_path());
		output << "\t\t\"normalTexture\": \"" << Utils::FixStringSlashesReturn(normal.string()) << "\",\n";

		std::filesystem::path specular = std::filesystem::relative(createInfo.specularPath, p.parent_path());
		output << "\t\t\"metalnessTexture\": \"" << Utils::FixStringSlashesReturn(specular.string()) << "\",\n";

		std::filesystem::path roughness = std::filesystem::relative(createInfo.roughnessPath, p.parent_path());
		output << "\t\t\"roughnessTexture\": \"" << Utils::FixStringSlashesReturn(roughness.string()) << "\"\n";

		output << "\t}\n";
	}

	output << "}";
	output.close();
	return false;
}

bool Materials::CreateStandardMaterial(StandardMaterialCreateInfo ci, std::string path) {
	return CreateStandardOrCutoutMaterial(ci, path, false);
}

bool Materials::CreateCutoutMaterial(StandardMaterialCreateInfo ci, std::string path) {
	return CreateStandardOrCutoutMaterial(ci, path, true);
}
