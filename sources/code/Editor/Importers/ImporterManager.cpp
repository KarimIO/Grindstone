#include <Common/ResourcePipeline/Uuid.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include "ImporterManager.hpp"

using namespace Grindstone::Importers;

static void ImportScene(std::filesystem::path& path) {
	Grindstone::MetaFile* metaFile = new Grindstone::MetaFile(path);
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}

	Grindstone::Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, Grindstone::AssetType::Scene);

	std::filesystem::path outputPath = Grindstone::Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();

	Grindstone::Editor::Manager::GetEngineCore().assetManager->QueueReloadAsset(Grindstone::AssetType::Scene, uuid);

	delete metaFile;
}

ImporterManager::ImporterManager() {
	AddImporterFactory("json", ImportScene);
}

bool ImporterManager::Import(std::filesystem::path& path) {
	ImporterFactory importerFactory = GetImporterFactoryByPath(path);
	if (importerFactory == nullptr) {
		return false;
	}

	importerFactory(path);
	return true;
}

void ImporterManager::AddImporterFactory(std::string extension, ImporterFactory importerFactory) {
	if (HasImporter(extension)) {
		return;
	}

	extensionsToImporterFactories[extension] = importerFactory;
}

void ImporterManager::RemoveImporterFactoryByExtension(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	if (extensionIterator != extensionsToImporterFactories.end()) {
		extensionsToImporterFactories.erase(extension);
	}
}

ImporterManager::ImporterFactory ImporterManager::GetImporterFactoryByExtension(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return (extensionIterator != extensionsToImporterFactories.end())
		? extensionIterator->second
		: ImporterFactory();
}

ImporterManager::ImporterFactory ImporterManager::GetImporterFactoryByPath(std::filesystem::path& path) {
	std::string extension = path.extension().string().substr(1);
	return GetImporterFactoryByExtension(extension);
}

bool ImporterManager::HasImporter(std::string& extension) {
	auto extensionIterator = extensionsToImporterFactories.find(extension);
	return extensionIterator != extensionsToImporterFactories.end();
}

bool ImporterManager::HasImporter(std::filesystem::path& path) {
	std::string extension = path.extension().string();
	if (extension.empty()) {
		return false;
	}

	std::string extensionWithoutDot = extension.substr(1);
	return HasImporter(extensionWithoutDot);
}
