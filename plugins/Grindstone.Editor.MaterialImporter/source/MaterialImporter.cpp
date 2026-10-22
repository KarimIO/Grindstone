#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include <rapidjson/document.h>

#include <EditorCommon/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <Editor/EditorManager.hpp>
#include <Grindstone.Editor.MaterialImporter/include/MaterialImporter.hpp>
#include <Logger.hpp>

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

	rapidjson::ParseResult parseResult = document.Parse(contentData.c_str());

	if (parseResult.IsError()) {
		rapidjson::GetParseErrorFunc GetParseError = rapidjson::GetParseErrorFunc();
		const RAPIDJSON_ERROR_CHARTYPE* errorCode = "Unknown Error";
		if (GetParseError != nullptr) {
			errorCode = GetParseError(parseResult.Code());
		}

		GPRINT_ERROR(LogSource::EditorImporter,
					 "Failed to import material '{}' - Could not parse file due to error '{}' with offset {}.",
					 inputPath.string(), errorCode, document.GetErrorOffset());
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

	AssetRegistry::Dependency shaderDependency;
	if (document.HasMember("shader")) {
		std::string shaderValue = document["shader"].GetString();

		Uuid shaderUuid;
		if (!Grindstone::Uuid::MakeFromString(shaderValue.c_str(), shaderUuid)) {
			shaderDependency = shaderUuid;
		}
		else {
			shaderDependency = std::move(shaderValue);
		}
	}
	else {
		GPRINT_ERROR(
			LogSource::EditorImporter,
					 "Failed to import material '{}' - Material is missing "
					 "'shader' parameter which is required.",
					 inputPath.string());
		return;
	}

	Grindstone::Uuid uuid = metaFile.GetOrCreateDefaultSubassetUuid(subassetName, AssetType::Material);
	metaFile.Save(materialImporterVersion);

	std::set<AssetRegistry::Dependency> dependencies;
	dependencies.insert(shaderDependency);
	assetRegistry.LinkHardDependencies(uuid, dependencies);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);
	assetManager.QueueReloadAsset(AssetType::Material, uuid);
}
