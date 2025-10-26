#include <imgui.h>

#include <Editor/Importers/ImporterManager.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/AssetTemplateRegistry.hpp>

#include "EditorPluginInterface.hpp"

using namespace Grindstone::Plugins;

struct ImGuiContext* EditorPluginInterface::GetImguiContext() const {
	return ImGui::GetCurrentContext();
}

Grindstone::Editor::Manager* EditorPluginInterface::GetEditorInstance() const {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	return &manager;
}

void EditorPluginInterface::MapExtensionToImporterType(const char* extension, Grindstone::HashedString importerType) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = manager.GetImporterManager();
	importerManager.MapExtensionToImporterType(extension, importerType);
}

void EditorPluginInterface::RegisterAssetImporter(
	Grindstone::HashedString importerType,
	Grindstone::Editor::ImporterData importerVersion
) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = manager.GetImporterManager();
	importerManager.AddImporterFactory(importerType, importerVersion);
}

void EditorPluginInterface::RegisterAssetTemplate(AssetType assetType, const char* name, const char* extension, const void* const sourcePtr, size_t sourceSize) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::AssetTemplateRegistry& assetTemplateRegistry = manager.GetAssetTemplateRegistry();
	assetTemplateRegistry.RegisterTemplate(assetType, name, extension, sourcePtr, sourceSize);
}

void EditorPluginInterface::UnmapExtensionToImporterType(const char* extension) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = manager.GetImporterManager();
	importerManager.UnmapExtensionToImporterType(extension);
}

void EditorPluginInterface::DeregisterAssetImporter(Grindstone::HashedString importerType) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Importers::ImporterManager& importerManager = manager.GetImporterManager();
	importerManager.RemoveImporterFactory(importerType);
}

void EditorPluginInterface::DeregisterAssetTemplate(AssetType assetType) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::AssetTemplateRegistry& assetTemplateRegistry = manager.GetAssetTemplateRegistry();
	assetTemplateRegistry.RemoveTemplate(assetType);
}

void EditorPluginInterface::RegisterThumbnailGenerator(AssetType assetType, bool(*fn)(Grindstone::Uuid)) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::ThumbnailManager& thumbnailManager = manager.GetThumbnailManager();
	thumbnailManager.RegisterGenerator(assetType, fn);
}

void EditorPluginInterface::DeregisterThumbnailGenerator(AssetType assetType, bool(*fn)(Grindstone::Uuid)) {
	Grindstone::Editor::Manager& manager = Grindstone::Editor::Manager::GetInstance();
	Grindstone::Editor::ThumbnailManager& thumbnailManager = manager.GetThumbnailManager();
	thumbnailManager.DeregisterGenerator(assetType, fn);
}
