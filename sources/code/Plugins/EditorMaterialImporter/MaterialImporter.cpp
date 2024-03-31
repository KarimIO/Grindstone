#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include <rapidjson/document.h>

#include <EngineCore/Assets/AssetManager.hpp>
#include "Editor/EditorManager.hpp"
#include "MaterialImporter.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

void MaterialImporter::Import(const std::filesystem::path& path) {
	metaFile = new MetaFile(path);

	std::string contentData = Grindstone::Utils::LoadFileText(path.string().c_str());
	rapidjson::Document document;
	document.Parse(contentData.data());

	std::string subassetName = "";
	if (document.HasMember("name")) {
		subassetName = document["name"].GetString();
	}
	else {
		subassetName = path.filename().string();
		size_t dotPos = subassetName.find('.');
		if (dotPos != std::string::npos) {
			subassetName = subassetName.substr(0, dotPos);
		}
	}

	uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Material);

	std::filesystem::path outputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();
	Editor::Manager::GetEngineCore().assetManager->QueueReloadAsset(AssetType::Material, uuid);
}

Uuid MaterialImporter::GetUuidAfterImport() const {
	return uuid;
}

void Editor::Importers::ImportMaterial(const std::filesystem::path& inputPath) {
	MaterialImporter materialImporter;
	materialImporter.Import(inputPath);
}
