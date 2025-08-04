#include <fstream>

#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/AssetRegistry.hpp>

#include <EditorModelImporter/include/ModelMaterialImporter.hpp>

using namespace Grindstone::Editor::Importers;

static void ImportTextureAndMakeOutputStr(Grindstone::Editor::AssetRegistry& assetRegistry, std::string textureName, std::filesystem::path& inputTexturePath, std::vector<std::string>& arr) {
	if (inputTexturePath == "") {
		return;
	}

	Grindstone::Uuid textureUuid = assetRegistry.Import(inputTexturePath);

	arr.push_back("\t\t\"" + textureName + "\": \"" + textureUuid.ToString());
}

static void CreateStandardOrCutoutMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, StandardMaterialCreateInfo& createInfo, std::filesystem::path outputPath, bool isCutout) {
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
		ImportTextureAndMakeOutputStr(assetRegistry, "albedoTexture", createInfo.albedoPath, textures);
		ImportTextureAndMakeOutputStr(assetRegistry, "normalTexture", createInfo.normalPath, textures);
		ImportTextureAndMakeOutputStr(assetRegistry, "metalnessTexture", createInfo.specularPath, textures);
		ImportTextureAndMakeOutputStr(assetRegistry, "roughnessTexture", createInfo.roughnessPath, textures);

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

void Grindstone::Editor::Importers::CreateStandardMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, StandardMaterialCreateInfo& ci, std::filesystem::path path) {
	CreateStandardOrCutoutMaterial(assetRegistry, ci, path, false);
}

void Grindstone::Editor::Importers::CreateCutoutMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, StandardMaterialCreateInfo& ci, std::filesystem::path path) {
	CreateStandardOrCutoutMaterial(assetRegistry, ci, path, true);
}
