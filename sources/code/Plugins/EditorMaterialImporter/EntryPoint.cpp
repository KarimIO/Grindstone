#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "MaterialImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_MATERIAL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("gmat", ImportMaterial);
		}
	}

	EDITOR_MATERIAL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("gmat");
		}
	}
}
