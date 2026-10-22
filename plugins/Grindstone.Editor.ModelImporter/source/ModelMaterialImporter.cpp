#include <fstream>

#include <Common/Logger.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/AssetRegistry.hpp>

#include <Grindstone.Editor.ModelImporter/include/ModelMaterialImporter.hpp>

using namespace Grindstone::Editor::Importers;

static void ImportTextureAndMakeOutputStr(Grindstone::Editor::AssetRegistry& assetRegistry, std::string textureName, std::filesystem::path& inputTexturePath, std::vector<std::string>& arr) {
	if (inputTexturePath == "" || !std::filesystem::exists(inputTexturePath)) {
		return;
	}

	Grindstone::Uuid textureUuid = assetRegistry.Import(inputTexturePath);

	arr.push_back("\t\t\"" + textureName + "\": \"" + textureUuid.ToString());
}

static Grindstone::Editor::AssetRegistry::Dependency CreateStandardOrCutoutMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, StandardMaterialCreateInfo& createInfo, std::filesystem::path outputPath, bool isCutout) {
	std::filesystem::path p = outputPath;

	std::ofstream output(outputPath);

	output << "{\n";

	output << "\t\"name\": \"" << createInfo.materialName << "\",\n";

	Grindstone::Editor::AssetRegistry::Dependency dependency;
	if (isCutout) {
		output << "\t\"shader\": \"cutout\",\n";
	}
	else {
		std::string uuidStr = "ad5ad34e-2017-487d-b2c3-489e26e63b3e";
		output << std::format("\t\"shader\": \"{}\",\n", uuidStr);
		Grindstone::Uuid outUuid;
		if (Grindstone::Uuid::MakeFromString(uuidStr.c_str(), outUuid)) {
			dependency = outUuid;
		}
		else {
			GPRINT_ERROR(Grindstone::LogSource::EditorImporter, "Unable to make uuid '{}'.", uuidStr.c_str());
		}
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

	return dependency;
}

Grindstone::Editor::AssetRegistry::Dependency Grindstone::Editor::Importers::CreateStandardMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, StandardMaterialCreateInfo& ci, std::filesystem::path path) {
	return CreateStandardOrCutoutMaterial(assetRegistry, ci, path, false);
}
