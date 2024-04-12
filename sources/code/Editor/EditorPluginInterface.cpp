#include <Editor/Importers/ImporterManager.hpp>
#include "EditorManager.hpp"
#include "EditorPluginInterface.hpp"
#include "AssetTemplateRegistry.hpp"

using namespace Grindstone::Plugins;

void EditorPluginInterface::RegisterAssetImporter(const char* extension, Grindstone::Importers::ImporterManager::ImporterFactory importer) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = manager.GetImporterManager();
	importerManager.AddImporterFactory(extension, importer);
}

void EditorPluginInterface::RegisterAssetTemplate(AssetType assetType, const char* name, const char* extension, const void* const sourcePtr, size_t sourceSize) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::AssetTemplateRegistry& assetTemplateRegistry = manager.GetAssetTemplateRegistry();
	assetTemplateRegistry.RegisterTemplate(assetType, name, extension, sourcePtr, sourceSize);
}

void EditorPluginInterface::DeregisterAssetImporter(const char* extension) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = manager.GetImporterManager();
	importerManager.RemoveImporterFactoryByExtension(extension);
}

void EditorPluginInterface::DeregisterAssetTemplate(AssetType assetType) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::AssetTemplateRegistry& assetTemplateRegistry = manager.GetAssetTemplateRegistry();
	assetTemplateRegistry.RemoveTemplate(assetType);
}
