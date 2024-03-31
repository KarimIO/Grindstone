#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>

#include "ModelImporter.hpp"

using namespace Grindstone;

extern "C" {
	EDITOR_MODEL_IMPORTER_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		AddImporterFactory("fbx", ImportModel);
		AddImporterFactory("dae", ImportModel);
		AddImporterFactory("obj", ImportModel);
	}

	EDITOR_MODEL_IMPORTER_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
	}
}
