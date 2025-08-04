#include <EditorAudioImporter/include/pch.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include <EditorAudioImporter/include/AudioImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

Grindstone::ConstHashedString audioImporterName("AudioImporter");

extern "C" {
	EDITOR_AUDIO_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter(audioImporterName, Grindstone::Editor::ImporterData{
				.importerVersion = audioImporterVersion,
				.factory = ImportAudio
			});
			editorPluginInterface->MapExtensionToImporterType("wav", audioImporterName);
		}
	}

	EDITOR_AUDIO_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->UnmapExtensionToImporterType("wav");
			editorPluginInterface->DeregisterAssetImporter(audioImporterName);
		}
	}
}
