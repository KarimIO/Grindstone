#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "Editor/EditorManager.hpp"
#include "MaterialImporter.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"

using namespace Grindstone;
using namespace Grindstone::Importers;

bool CreateStandardOrCutoutMaterial(StandardMaterialCreateInfo createInfo, std::filesystem::path outputPath, bool isCutout) {
	std::filesystem::path p = outputPath;
	std::filesystem::path pathToShader = std::filesystem::relative("../assets/test", p.parent_path());

	std::ofstream output(outputPath);

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

bool Importers::CreateStandardMaterial(StandardMaterialCreateInfo ci, std::filesystem::path path) {
	bool response = CreateStandardOrCutoutMaterial(ci, path, false);
	ImportMaterial(path);
	return response;
}

bool Importers::CreateCutoutMaterial(StandardMaterialCreateInfo ci, std::filesystem::path path) {
	bool response = CreateStandardOrCutoutMaterial(ci, path, true);
	ImportMaterial(path);
	return response;
}

void MaterialImporter::Import(std::filesystem::path& path) {
	metaFile = new MetaFile(path);
	std::string subassetName = "material";
	Uuid uuid = metaFile->GetOrCreateSubassetUuid(subassetName);

	std::filesystem::copy(path, std::string("../compiledAssets/") + uuid.ToString());
	metaFile->Save();
}

void Importers::ImportMaterial(std::filesystem::path& inputPath) {
	MaterialImporter TextureImporter;
	TextureImporter.Import(inputPath);
}
