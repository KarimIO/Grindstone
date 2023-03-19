#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "Editor/EditorManager.hpp"
#include "MaterialImporter.hpp"
#include "TextureImporter.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"

using namespace Grindstone;
using namespace Grindstone::Importers;

std::string ImportTextureAndMakeOutputStr(std::string textureName, std::filesystem::path& inputTexturePath, bool isLast) {
	if (inputTexturePath == "") {
		return "";
	}

	TextureImporter textureImporter;
	textureImporter.Import(inputTexturePath);

	return "\t\t\"" + textureName + "\": \"" + textureImporter.GetUuid().ToString() +
		(isLast ? "\"\n" : "\",\n");
}

void CreateStandardOrCutoutMaterial(StandardMaterialCreateInfo& createInfo, std::filesystem::path outputPath, bool isCutout) {
	std::filesystem::path p = outputPath;

	std::ofstream output(outputPath);

	output << "{\n";
	
	output << "\t\"name\": \"" << createInfo.materialName << "\",\n";

	if (isCutout) {
		output << "\t\"shader\": \"cutout\",\n";
	}
	else {
		output << "\t\"shader\": \"ad5ad34e-2017-487d-b2c3-489e26e63b3e\",\n";
	}

	output << "\t\"parameters\": {\n";
	{
		output << "\t\t\"color\": [1, 1, 1, 1]\n";
	}
	output << "\t},\n";
	
	output << "\t\"samplers\": {\n";
	{
		output << ImportTextureAndMakeOutputStr("albedoTexture", createInfo.albedoPath, false);
		output << ImportTextureAndMakeOutputStr("normalTexture", createInfo.normalPath, false);
		output << ImportTextureAndMakeOutputStr("metalnessTexture", createInfo.specularPath, false);
		output << ImportTextureAndMakeOutputStr("roughnessTexture", createInfo.roughnessPath, true);
	}
	output << "\t}\n";

	output << "}";
	output.close();
}

void Importers::CreateStandardMaterial(StandardMaterialCreateInfo& ci, std::filesystem::path path) {
	CreateStandardOrCutoutMaterial(ci, path, false);
}

void Importers::CreateCutoutMaterial(StandardMaterialCreateInfo& ci, std::filesystem::path path) {
	CreateStandardOrCutoutMaterial(ci, path, true);
}

void MaterialImporter::Import(std::filesystem::path& path) {
	metaFile = new MetaFile(path);
	std::string subassetName = "material";
	uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName);

	std::filesystem::path outputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();
}

Uuid MaterialImporter::GetUuidAfterImport() {
	return uuid;
}

Uuid Importers::ImportMaterial(std::filesystem::path& inputPath) {
	MaterialImporter materialImporter;
	materialImporter.Import(inputPath);
	return materialImporter.GetUuidAfterImport();
}
