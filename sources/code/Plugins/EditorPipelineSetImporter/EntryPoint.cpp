#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "PipelineSetImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Editor::Importers;

Grindstone::ConstHashedString gpsetImporterName("MaterialImporter");

extern "C" {
	EDITOR_PIPELINESET_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter(gpsetImporterName, Grindstone::Editor::ImporterData{
				.importerVersion = pipelineSetImporterVersion,
				.factory = ImportShadersFromGlsl
			});
			editorPluginInterface->MapExtensionToImporterType("gpset", gpsetImporterName);
		}
	}

	EDITOR_PIPELINESET_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->UnmapExtensionToImporterType("gpset");
			editorPluginInterface->DeregisterAssetImporter(gpsetImporterName);
		}
	}
}
