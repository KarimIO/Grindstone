#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include <rapidjson/document.h>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/EditorManager.hpp>
#include <Grindstone.Editor.MaterialImporter/include/MaterialImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

void Editor::Importers::ImportMaterial(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& inputPath) {
	Grindstone::Editor::MetaFile metaFile = assetRegistry.GetMetaFileByPath(inputPath);

	std::string contentData = Grindstone::Utils::LoadFileText(inputPath.string().c_str());
	rapidjson::Document document;
	if (document.Parse(contentData.data()).GetParseError()) {
		// TODO: Print error
		return;
	}

	std::string subassetName = "";
	if (document.HasMember("name")) {
		subassetName = document["name"].GetString();
	}
	else {
		subassetName = inputPath.filename().string();
		size_t dotPos = subassetName.find('.');
		if (dotPos != std::string::npos) {
			subassetName = subassetName.substr(0, dotPos);
		}
	}

	Grindstone::Uuid uuid = metaFile.GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Material);
	metaFile.Save(materialImporterVersion);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);
	assetManager.QueueReloadAsset(AssetType::Material, uuid);
	std::filesystem::last_write_time(outputPath, std::filesystem::file_time_type::clock::now());
}
