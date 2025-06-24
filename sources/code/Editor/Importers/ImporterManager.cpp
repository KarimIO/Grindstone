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

static void ImportCopyFile(Grindstone::AssetType assetType, AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path, Grindstone::Editor::ImporterVersion assetVersion) {
	Grindstone::Editor::MetaFile* metaFile = assetRegistry.GetMetaFileByPath(path);
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Grindstone::Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, assetType);

	std::filesystem::path outputPath = Grindstone::Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save(assetVersion);

	assetManager.QueueReloadAsset(assetType, uuid);

	delete metaFile;
}

const Grindstone::Editor::ImporterVersion gsceneAssetVersion = 1;
const Grindstone::Editor::ImporterVersion ddsAssetVersion = 1;

static void ImportScene(AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path) {
	ImportCopyFile(Grindstone::AssetType::Scene, assetRegistry, assetManager, path, gsceneAssetVersion);
}

static void ImportDdsTexture(AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path) {
	ImportCopyFile(Grindstone::AssetType::Texture, assetRegistry, assetManager, path, ddsAssetVersion);
}

ImporterManager::ImporterManager() {
	AddImporterFactory("gscene", ImportScene, gsceneAssetVersion);
	AddImporterFactory("dds", ImportDdsTexture, ddsAssetVersion);
}

bool ImporterManager::Import(const std::filesystem::path& path) {
	ImporterFactory importerFactory = GetImporterFactoryByPath(path);
	if (importerFactory == nullptr) {
		return false;
	}

	AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();
	AssetManager& assetManager = *Editor::Manager::GetEngineCore().assetManager;
	importerFactory(assetRegistry, assetManager, path);
	return true;
}

void ImporterManager::AddImporterFactory(const std::string& extension, ImporterFactory importerFactory, uint32_t importerVersion) {
	if (HasImporter(extension)) {
		return;
	}

	extensionsToImporterFactories[extension] = ImporterData{
		importerVersion,
		importerFactory
	};
}

void ImporterManager::RemoveImporterFactoryByExtension(const std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	if (extensionIterator != extensionsToImporterFactories.end()) {
		extensionsToImporterFactories.erase(extension);
	}
}

Grindstone::Editor::ImporterFactory ImporterManager::GetImporterFactoryByExtension(const std::string& extension) const {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return (extensionIterator != extensionsToImporterFactories.end())
		? extensionIterator->second.factory
		: nullptr;
}

Grindstone::Editor::ImporterFactory ImporterManager::GetImporterFactoryByPath(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string().substr(1);
	return GetImporterFactoryByExtension(extension);
}

Grindstone::Editor::ImporterVersion ImporterManager::GetImporterVersion(const std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return (extensionIterator != extensionsToImporterFactories.end())
		? extensionIterator->second.importerVersion
		: 0;
}

Grindstone::Editor::ImporterVersion ImporterManager::GetImporterVersion(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string();
	if (extension.empty()) {
		return false;
	}

	const std::string extensionWithoutDot = extension.substr(1);
	return HasImporter(extensionWithoutDot);
}

bool ImporterManager::HasImporter(const std::string& extension) const {
	const auto extensionIterator = extensionsToImporterFactories.find(extension);
	return extensionIterator != extensionsToImporterFactories.end();
}

bool ImporterManager::HasImporter(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string();
	if (extension.empty()) {
		return false;
	}

	const std::string extensionWithoutDot = extension.substr(1);
	return HasImporter(extensionWithoutDot);
}
