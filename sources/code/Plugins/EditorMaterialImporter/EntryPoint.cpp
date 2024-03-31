#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "MaterialImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

extern "C" {
	EDITOR_MATERIAL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorRegisterAssetImporter("gmat", ImportMaterial);
	}

	EDITOR_MATERIAL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->EditorDeregisterAssetImporter("gmat");
	}
}
