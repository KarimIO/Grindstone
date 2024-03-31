#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "MaterialImporter.hpp"

using namespace Grindstone;

extern "C" {
	EDITOR_MATERIAL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->AddEditorImporterFactory("gmat", ImportMaterial);
		pluginInterface->AddEditorImporterFactory("gmat", ImportMaterial);
	}

	EDITOR_MATERIAL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
