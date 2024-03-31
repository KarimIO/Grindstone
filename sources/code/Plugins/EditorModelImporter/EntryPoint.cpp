#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "ModelImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_MODEL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorRegisterAssetImporter("fbx", ImportModel);
		pluginInterface->EditorRegisterAssetImporter("dae", ImportModel);
		pluginInterface->EditorRegisterAssetImporter("obj", ImportModel);
	}

	EDITOR_MODEL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorDeregisterAssetImporter("fbx");
		pluginInterface->EditorDeregisterAssetImporter("dae");
		pluginInterface->EditorDeregisterAssetImporter("obj");
	}
}
