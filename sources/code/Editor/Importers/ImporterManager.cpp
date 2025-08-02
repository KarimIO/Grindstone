#include <filesystem>
#include <string>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/AssetRegistry.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include "ImporterManager.hpp"

using namespace Grindstone::Assets;
using namespace Grindstone::Editor;
using namespace Grindstone::Importers;

static void ImportCopyFile(Grindstone::AssetType assetType, AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& inputPath, Grindstone::Editor::ImporterVersion assetVersion) {
	Grindstone::Editor::MetaFile metaFile = assetRegistry.GetMetaFileByPath(inputPath);
	std::string subassetName = inputPath.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Grindstone::Uuid uuid = metaFile.GetOrCreateDefaultSubassetUuid(subassetName, assetType);

	std::filesystem::path outputPath = Grindstone::Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile.Save(assetVersion);
	std::filesystem::last_write_time(outputPath, std::filesystem::file_time_type::clock::now());

	assetManager.QueueReloadAsset(assetType, uuid);
}

const Grindstone::Editor::ImporterVersion gsceneAssetVersion = 1;
const Grindstone::Editor::ImporterVersion ddsAssetVersion = 1;

static void ImportScene(AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path) {
	ImportCopyFile(Grindstone::AssetType::Scene, assetRegistry, assetManager, path, gsceneAssetVersion);
}

static void ImportDdsTexture(AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path) {
	ImportCopyFile(Grindstone::AssetType::Texture, assetRegistry, assetManager, path, ddsAssetVersion);
}

Grindstone::ConstHashedString sceneImporterName("SceneImporter");
Grindstone::ConstHashedString ddsImporterName("DdsImporter");

void ImporterManager::Initialize() {
	AddImporterFactory(sceneImporterName, ImporterData{ .importerVersion = gsceneAssetVersion, .factory = ImportScene });
	AddImporterFactory(ddsImporterName, ImporterData{ .importerVersion = ddsAssetVersion, .factory = ImportDdsTexture });
	MapExtensionToImporterType("gscene", sceneImporterName);
	MapExtensionToImporterType("dds", ddsImporterName);
}

bool ImporterManager::Import(const std::filesystem::path& path) {
	ImporterFactory importerFactory = GetImporterFactoryByPath(path).factory;
	if (importerFactory == nullptr) {
		return false;
	}

	AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();
	AssetManager& assetManager = *Editor::Manager::GetEngineCore().assetManager;
	importerFactory(assetRegistry, assetManager, path);
	return true;
}

void ImporterManager::AddImporterFactory(Grindstone::HashedString importerType, Grindstone::Editor::ImporterData importerData) {
	if (HasImporter(importerType)) {
		return;
	}

	importerFactoriesMap[importerType] = importerData;
}

void ImporterManager::MapExtensionToImporterType(const std::string& extension, Grindstone::HashedString importerType) {
	extensionsToImporterFactories[extension] = importerType;
}

void ImporterManager::RemoveImporterFactory(Grindstone::HashedString importerType) {
	auto extensionIterator = importerFactoriesMap.find(importerType);
	if (extensionIterator != importerFactoriesMap.end()) {
		importerFactoriesMap.erase(extensionIterator);
	}
}

void ImporterManager::UnmapExtensionToImporterType(const std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	if (extensionIterator != extensionsToImporterFactories.end()) {
		extensionsToImporterFactories.erase(extension);
	}
}

Grindstone::Editor::ImporterData ImporterManager::GetImporterFactoryByName(Grindstone::HashedString importerType) const {
	auto extensionIterator = importerFactoriesMap.find(importerType);
	return (extensionIterator != importerFactoriesMap.end())
		? extensionIterator->second
		: Grindstone::Editor::ImporterData{};
}


Grindstone::Editor::ImporterData ImporterManager::GetImporterFactoryByExtension(const std::string& extension) const {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	if (extensionIterator == extensionsToImporterFactories.end()) {
		return {};
	}

	return GetImporterFactoryByName(extensionIterator->second);
}

Grindstone::Editor::ImporterData ImporterManager::GetImporterFactoryByPath(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string();
	if (extension.empty()) {
		return {};
	}

	const std::string extensionWithoutDot = extension.substr(1);
	return GetImporterFactoryByExtension(extensionWithoutDot);
}

Grindstone::Editor::ImporterVersion ImporterManager::GetImporterVersion(Grindstone::HashedString importerType) const {
	auto extensionIterator = importerFactoriesMap.find(importerType);
	return (extensionIterator != importerFactoriesMap.end())
		? extensionIterator->second.importerVersion
		: 0;
}

Grindstone::Editor::ImporterVersion ImporterManager::GetImporterVersionByExtension(const std::string& extension) const {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return (extensionIterator != extensionsToImporterFactories.end())
		? GetImporterVersion(extensionIterator->second)
		: 0;
}

Grindstone::Editor::ImporterVersion ImporterManager::GetImporterVersionByPath(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string();
	if (extension.empty()) {
		return 0;
	}

	const std::string extensionWithoutDot = extension.substr(1);
	return GetImporterVersionByExtension(extensionWithoutDot);
}

bool ImporterManager::HasImporter(Grindstone::HashedString importerType) const {
	const auto extensionIterator = importerFactoriesMap.find(importerType);
	return extensionIterator != importerFactoriesMap.end();
}

bool ImporterManager::HasImporterForExtension(const std::string& extension) const {
	const auto extensionIterator = extensionsToImporterFactories.find(extension);
	return extensionIterator != extensionsToImporterFactories.end();
}

bool ImporterManager::HasImporterForPath(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string();
	if (extension.empty()) {
		return false;
	}

	const std::string extensionWithoutDot = extension.substr(1);
	return HasImporterForExtension(extensionWithoutDot);
}
