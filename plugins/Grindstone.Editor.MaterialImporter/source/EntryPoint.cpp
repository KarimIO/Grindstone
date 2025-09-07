#include <Grindstone.Editor.MaterialImporter/include/pch.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Grindstone.Editor.MaterialImporter/include/MaterialImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

Grindstone::ConstHashedString materialImporterName("MaterialImporter");

extern "C" {
	EDITOR_MATERIAL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter(materialImporterName, Grindstone::Editor::ImporterData{
				.importerVersion = materialImporterVersion,
				.factory = ImportMaterial
			});
			editorPluginInterface->MapExtensionToImporterType("gmat", materialImporterName);
		}
	}

	EDITOR_MATERIAL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->UnmapExtensionToImporterType("gmat");
			editorPluginInterface->DeregisterAssetImporter(materialImporterName);
		}
	}
}
