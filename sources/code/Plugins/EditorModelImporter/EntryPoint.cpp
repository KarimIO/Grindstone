#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <Editor/EditorPluginInterface.hpp>

#include "ModelImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_MODEL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->RegisterAssetImporter("fbx", ImportModel);
			editorPluginInterface->RegisterAssetImporter("dae", ImportModel);
			editorPluginInterface->RegisterAssetImporter("obj", ImportModel);
		}
	}

	EDITOR_MODEL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface != nullptr) {
			editorPluginInterface->DeregisterAssetImporter("fbx");
			editorPluginInterface->DeregisterAssetImporter("dae");
			editorPluginInterface->DeregisterAssetImporter("obj");
		}
	}
}
