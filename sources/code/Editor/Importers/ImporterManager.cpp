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

static void ImportScene(AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path) {
	Grindstone::Editor::MetaFile* metaFile = assetRegistry.GetMetaFileByPath(path);
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Grindstone::Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, Grindstone::AssetType::Scene);

	std::filesystem::path outputPath = Grindstone::Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();

	assetManager.QueueReloadAsset(Grindstone::AssetType::Scene, uuid);

	delete metaFile;
}

static void ImportDdsTexture(AssetRegistry& assetRegistry, AssetManager& assetManager, const std::filesystem::path& path) {
	Grindstone::Editor::MetaFile* metaFile = assetRegistry.GetMetaFileByPath(path);
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Grindstone::Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, Grindstone::AssetType::Scene);

	std::filesystem::path outputPath = Grindstone::Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();

	assetManager.QueueReloadAsset(Grindstone::AssetType::Texture, uuid);

	delete metaFile;
}

ImporterManager::ImporterManager() {
	AddImporterFactory("json", ImportScene);
	AddImporterFactory("dds", ImportDdsTexture);
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

void ImporterManager::AddImporterFactory(const std::string& extension, ImporterFactory importerFactory) {
	if (HasImporter(extension)) {
		return;
	}

	extensionsToImporterFactories[extension] = importerFactory;
}

void ImporterManager::RemoveImporterFactoryByExtension(const std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	if (extensionIterator != extensionsToImporterFactories.end()) {
		extensionsToImporterFactories.erase(extension);
	}
}

ImporterManager::ImporterFactory ImporterManager::GetImporterFactoryByExtension(const std::string& extension) const {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return (extensionIterator != extensionsToImporterFactories.end())
		? extensionIterator->second
		: nullptr;
}

ImporterManager::ImporterFactory ImporterManager::GetImporterFactoryByPath(const std::filesystem::path& path) const {
	const std::string extension = path.extension().string().substr(1);
	return GetImporterFactoryByExtension(extension);
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
