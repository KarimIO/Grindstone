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

void ImportTextureAndMakeOutputStr(std::string textureName, std::filesystem::path& inputTexturePath, std::vector<std::string>& arr) {
	if (inputTexturePath == "") {
		return;
	}

	TextureImporter textureImporter;
	textureImporter.Import(inputTexturePath);

	arr.push_back("\t\t\"" + textureName + "\": \"" + textureImporter.GetUuid().ToString());
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
		std::vector<std::string> textures;
		ImportTextureAndMakeOutputStr("albedoTexture", createInfo.albedoPath, textures);
		ImportTextureAndMakeOutputStr("normalTexture", createInfo.normalPath, textures);
		ImportTextureAndMakeOutputStr("metalnessTexture", createInfo.specularPath, textures);
		ImportTextureAndMakeOutputStr("roughnessTexture", createInfo.roughnessPath, textures);

		for (size_t i = 0; i < textures.size(); ++i) {
			output << textures[i];

			if (i != textures.size() - 1) {
				output << "\",\n";
			}
			else {
				output << "\"\n";
			}
		}
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
	uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Material);

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
